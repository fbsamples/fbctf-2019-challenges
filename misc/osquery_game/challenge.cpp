/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#undef NDEBUG

#include <osquery/extensions.h>
#include <osquery/logger.h>
#include <osquery/sql.h>
#include <osquery/filesystem.h>
#include <osquery/tables.h>
#include <osquery/registry.h>

#include <osquery/core/conversions.h>

#include <mutex>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <future>

#include "emoji.h"

namespace osquery {
/**
 * @brief Create the external SQLite implementation wrapper.
 *
 * Anything built with only libosquery and not the 'additional' library will
 * not include a native SQL implementation. This applies to extensions and
 * separate applications built with the osquery SDK.
 *
 * The ExternalSQLPlugin is a wrapper around the SQLite API, which forwards
 * calls to an osquery extension manager (core).
 */
REGISTER_INTERNAL(ExternalSQLPlugin, "sql", "sql");

/**
 * @brief Mimic the REGISTER macro, extensions should use this helper.
 *
 * The SDK does not provide a REGISTER macro for modules or extensions.
 * Tools built with the osquery SDK should use REGISTER_EXTERNAL to add to
 * their own 'external' registry. This registry will broadcast to the osquery
 * extension manager (core) in an extension.
 *
 * osquery 'modules' should not construct their plugin registrations in
 * global scope (global construction time). Instead they should use the
 * module call-in well defined symbol, declare their SDK constraints, then
 * use the REGISTER_MODULE call within `initModule`.
 */
#define REGISTER_EXTERNAL(class_name, registry_name, plugin_name)              \
  namespace registries {                                                       \
  const ::osquery::registries::PI<class_name>                                  \
      k##ExtensionRegistryItem##class_name(registry_name, plugin_name, false); \
  }
}

using namespace osquery;

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;
namespace errc = boost::system::errc;

// bug 2: overflow this.
uint8_t kDay = 0;

enum Icon: size_t {
  BLANK = 0,
  TRACTOR = 1,
  PLOWED = 2,
  PIG = 3,
  BUCKET = 4,
  SHEEP = 5,
  SEEDLING = 6,
  DEAD = 7,
  PLANT = 8,
  SUNNYFACE = 9,
  SUNFLOWER = 10,
};

#define A_MOVE "move"
#define A_WATER "water"
#define A_PLANT "plant"
#define A_SHOW "show"
#define A_PICKUP "pickup"

struct Cell {
  Icon i;
  size_t p;
};

std::vector<struct Cell> kCells;
bool kHaveWater = false;
bool kHavePlant = false;
bool kSheepGood = false;

std::map<Icon, std::string> kEmoji = {
  {BLANK, emojicpp::emojize(":herb:")},
  {TRACTOR, emojicpp::emojize(":tractor:")},
  {PLOWED, emojicpp::emojize(":white_large_square:")},
  {PIG, emojicpp::emojize(":pig:")},
  {BUCKET, emojicpp::emojize(":potable_water:")},
  {SHEEP, emojicpp::emojize(":sheep:")},
  {SEEDLING, emojicpp::emojize(":seedling:")},
  {DEAD, "🥀"},
  {PLANT, emojicpp::emojize(":cherries:")},
  {SUNNYFACE, emojicpp::emojize(":sun_with_face:")},
  {SUNFLOWER, emojicpp::emojize(":sunflower:")},
};

size_t getX(size_t pos) {
  auto y = (pos & 0xF0) >> 4;
  return y;
}

size_t getY(size_t pos) {
  auto x = pos & 0xF;
  return x;
}

size_t getPos(size_t x, size_t y) {
  return (x * 16) + y;
}

bool nextTo(size_t a, size_t b) {
  int dy, dx;

  dx = getX(a) - getX(b);
  dy = getY(a) - getY(b);
  return ((dx >= -1 && dx <= 1) || (dy >= -1 && dy <= 1));
}

size_t invertPos(size_t pos) {
  // People will enter YX vs XY.
  auto x = getX(pos);
  auto y = getY(pos);
  return getPos(y, x);
}

class FarmEmojiTable : public TablePlugin {
 private:
  TableColumns columns() const {
    return {
        std::make_tuple("emoji", TEXT_TYPE, ColumnOptions::DEFAULT),
        std::make_tuple("meaning", TEXT_TYPE, ColumnOptions::DEFAULT),
    };
  }

  QueryData generate(QueryContext& ctx) {
    QueryData results;

    results.push_back({{"emoji", emojicpp::emojize(":herb:")}, {"meaning", "weeds"}});
    results.push_back({{"emoji", emojicpp::emojize(":tractor:")}, {"meaning", "tractor"}});
    results.push_back({{"emoji", emojicpp::emojize(":white_large_square:")}, {"meaning", "plowed plot, plant seeds here"}});
    results.push_back({{"emoji", emojicpp::emojize(":pig:")}, {"meaning", "pig"}});
    results.push_back({{"emoji", emojicpp::emojize(":potable_water:")}, {"meaning", "water pail, pick it up, use it to water planted seeds"}});
    results.push_back({{"emoji", emojicpp::emojize(":sheep:")}, {"meaning", "sheep"}});
    results.push_back({{"emoji", emojicpp::emojize(":seedling:")}, {"meaning", "seedling that needs water"}});
    results.push_back({{"emoji", "🥀"}, {"meaning", "a dead plant"}});
    results.push_back({{"emoji",  emojicpp::emojize(":cherries:")}, {"meaning", "plant"}});
    results.push_back({{"emoji",  emojicpp::emojize(":sunflower:")}, {"meaning", "sunflower"}});
    return results;
  }
};

class FarmActionsTable : public TablePlugin {
 private:
  TableColumns columns() const {
    return {
        std::make_tuple("action", TEXT_TYPE, ColumnOptions::DEFAULT),
        std::make_tuple("description", TEXT_TYPE, ColumnOptions::DEFAULT),
    };
  }

  QueryData generate(QueryContext& ctx) {
    QueryData results;

    results.push_back({{"action", A_SHOW}, {"description", "Default action, shows the farm."}});
    results.push_back({{"action", A_MOVE " [src] [dst]"}, {"description", "Requests to move animal in SRC field to DST field."}});
    results.push_back({{"action", A_PICKUP " [src]"}, {"description", "Pickup item in SRC field."}});
    results.push_back({{"action", A_WATER " [...dst]"}, {"description", "Water planted herb located at DST."}});
    results.push_back({{"action", A_PLANT " [...dst]"}, {"description", "Plant a herb in the plowed DST."}});

    return results;
  }
};

class FarmMailboxTable : public TablePlugin {
 private:
  TableColumns columns() const {
    return {
        std::make_tuple("from", TEXT_TYPE, ColumnOptions::DEFAULT),
        std::make_tuple("message", TEXT_TYPE, ColumnOptions::DEFAULT),
        std::make_tuple("done", TEXT_TYPE, ColumnOptions::DEFAULT),
    };
  }

  QueryData generate(QueryContext& ctx) {
    QueryData results;

    std::string done = ((kSheepGood) ? "yes" : "no");
    results.push_back({{"from", "Town Mayor"}, {"message", "The sheep wants to be next to the pig. Please move him, but be careful, if he sees you he will run away in less than a second, you need to move fast."}, {"done", done}});

    size_t seedling = 0;
    size_t plants = 0;
    for (auto& c : kCells) {
      if (c.i == PLANT) {
        plants++;
      } else if (c.i == SEEDLING) {
        seedling++;
      }
    }
    done = ((plants > 0) ? "yes" : "no");
    results.push_back({{"from", "Town Mayor"}, {"message", "Please water something that you have planted. You need to pickup a pail first. The sheep was playing with the water pail, if you move him next to his friend he may give it back."}, {"done", done}});

    done = ((kHavePlant) ? "yes" : "no");
    results.push_back({{"from", "Town Mayor"}, {"message", "Please pick something that you have grown. Wait a day after planting a seed and watering then pickup your plants."}, {"done", done}});
    results.push_back({{"from", "Town Mayor"}, {"message", "Weeds grow the first day of each season. Be careful, seeds and small plants will be overtaken."}, {"done", "yes"}});

    return results;
  }
};

std::set<size_t> getDstPositions(const ConstraintList& dst) {
  std::set<size_t> all;

  auto equals = dst.getAll(EQUALS);
  for (const auto e : equals) {
    auto op = tryTo<size_t>(e);
    if (op) {
      all.insert(op.take());
    }
  }

  size_t max_gte = 0;
  auto gt = dst.getAll(GREATER_THAN);
  auto gte = dst.getAll(GREATER_THAN_OR_EQUALS);
  if (gt.size() > 0 || gte.size() > 0) {
    for (auto e : gt) {
      auto op = tryTo<size_t>(e);
      if (op && op.take()+1 > max_gte) {
        max_gte = op.take()+1;
      }
    }

    for (auto e : gte) {
      auto op = tryTo<size_t>(e);
      if (op && op.take() > max_gte) {
        max_gte = op.take();
      }
    }
  }

  {
    std::stringstream stream;
    stream << std::hex << max_gte;
  }

  size_t min_lte = 0;
  auto lt = dst.getAll(LESS_THAN);
  auto lte = dst.getAll(LESS_THAN_OR_EQUALS);
  if (lt.size() > 0 || lte.size() > 0) {
    for (auto e : lt) {
      auto op = tryTo<size_t>(e);
      if (op && op.take()-1 > min_lte) {
        min_lte = op.take()-1;
      }
    }

    for (auto e : lte) {
      auto op = tryTo<size_t>(e);
      if (op && op.take() > min_lte) {
        min_lte = op.take();
      }
    }
  }

  {
    std::stringstream stream;
    stream << std::hex << min_lte;
  }

  if (min_lte > 0) {
    for (auto i = max_gte; i <= min_lte; i++) {
      all.insert(i);
    }
  } else if (gt.size() > 0 || gte.size() > 0) {
    for (auto i = max_gte; i <= 255; i++) {
      all.insert(i);
    }
  }

  return all;
}

size_t tryPlant(std::set<size_t> dsts) {
  size_t ret = 0;
  for (auto d : dsts) {
    for (auto& c : kCells) {
      if (c.p == invertPos(d) && c.i == PLOWED) {
        //LOG(WARNING) << "Match for plow " << d;
        ret = d;
        c.i = SEEDLING;
      } else if (c.p == invertPos(d)) {
        LOG(WARNING) << "You cannot plant on this src column, it is not plowed.";
      }
    }
  }
  return ret;
}

size_t tryWater(std::set<size_t> dsts) {
  size_t ret = 0;
  for (auto d : dsts) {
    for (auto& c : kCells) {
      if (c.p == invertPos(d) && c.i == SEEDLING) {
        ret = d;
        // bug 4: (unintentional) do not listen to quest quest! the cake is a lie!
        c.i = PLANT;
      }
    }
  }
  return ret;
}

bool tryValidMove(size_t src, size_t dst) {
  static bool kPigOrSheepMoved = false;

  bool srcOK = false;
  if (getX(dst) > 16 || getY(dst) > 16) {
    return false;
  }

  size_t i = 0, index = 0;
  for (const auto c : kCells) {
    if (c.p == invertPos(dst)) {
      return false;
      break;
    } else if (c.p == invertPos(src)) {
      if (c.i == PIG || c.i == SHEEP || c.i == TRACTOR || c.i == SUNFLOWER) {
        index = i;
        srcOK = true;
      }
    }
    i++;
  }

  if (srcOK) {
    if ((kCells[index].i == PIG || kCells[index].i == SHEEP) && !kPigOrSheepMoved) {
      kCells.push_back({BUCKET, kCells[index].p});
      kPigOrSheepMoved = true;
    }
    kCells[index].p = invertPos(dst);
  }

  return srcOK;
}

bool tryPickup(size_t src) {
  size_t i = 0, index = 0;
  bool pickupOK = false;
  bool pickupWater = false;

  std::set<size_t> pickups;
  for (auto c : kCells) {
    if (c.p == invertPos(src) && c.i == BUCKET) {
      index = i;
      pickupOK = true;
      pickupWater = true;
      break;
    } else if (c.p == invertPos(src) && c.i == SUNFLOWER) {
      index = i;
      pickupOK = true;
      break;
    } else if (c.p == invertPos(src) && c.i == PLANT) {
      index = i;
      pickupOK = true;
      kHavePlant = true;
    }
    i++;
  }

  if (pickupOK) {
    if (pickupWater) {
      LOG(WARNING) << "You have a water pail in your inventory. You can now use the 'water' action to water seeds.";
      kHaveWater = true;
    }
    kCells.erase(kCells.begin()+index);
  }

  return pickupOK;
}

std::string showFarm() {
  std::string farm;

  for (const auto c : kCells) {
    std::stringstream stream;
    stream << std::hex << c.p;
  }

  farm += "  ";
  for (size_t i = 0; i <= 0xF; i++) {
    if (i < 10) {
      farm += (char)i+48;
    } else {
      farm += (char)i+65-10;
    }
    farm += ' ';
  }
  farm += '\n';
  for (size_t y = 0; y <= 0xF; y++) {
    if (y < 10) {
      farm += (char)y+48;
    } else {
      farm += (char)y+65-10;
    }

    for (size_t x = 0; x <= 0xF; x++) {
      bool icon = false;
      auto pos = getPos(x, y);
      for (const auto c : kCells) {
        if (c.p == pos) {
          farm += kEmoji[c.i];
          icon = true;
          break;
        }
      }
      if (!icon) {
        farm += kEmoji[BLANK];
      }
    }
    farm += "\n";
  }
  return farm;
}

void sheepRunsAway() {
  // bug 1: race against this.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  struct Cell *sheep = nullptr;
  struct Cell *pig = nullptr;
  for (auto& c : kCells) {
    if (c.i == SHEEP) {
      sheep = &c;
    } else if (c.i == PIG) {
      pig = &c;
    }
  }

  if (sheep && pig) {
    if (!nextTo(sheep->p, pig-> p)) {
      LOG(ERROR) << "The sheep was not next to his friend the pig. He saw you and ran away scared.";
      LOG(ERROR) << "You failed a quest and cannot win the game. Please retry.";
      sheep->i = BLANK;
    } else {
      kSheepGood = true;
    }
  }
}

std::vector<std::future<void>> kThreads;

class FarmTable : public TablePlugin {
 private:
  TableColumns columns() const {
    return {
        std::make_tuple("farm", TEXT_TYPE, ColumnOptions::DEFAULT),
        std::make_tuple("action", TEXT_TYPE, ColumnOptions::ADDITIONAL),
        std::make_tuple("src", INTEGER_TYPE, ColumnOptions::ADDITIONAL),
        std::make_tuple("dst", INTEGER_TYPE, ColumnOptions::ADDITIONAL),
    };
  }

  QueryData generate(QueryContext& ctx) {
    QueryData results;

    kDay++;

    if (kDay == 1) {
      bool weeds = false;
      for (auto& c : kCells) {
        if (c.i == PLANT || c.i == SEEDLING) {
          weeds = true;
          c.i = BLANK;
        }
      }
      if (weeds) {
        LOG(WARNING) << "Some plants or seedlings were overtaken by weeds.";
      }
    }

    if (kDay > 5) {
      LOG(ERROR) << "The farming season is over.";
      return {};
    }

    Row r;

    LOG(WARNING) << "Good morning! It is day " << (int)kDay << "/256 " << kEmoji[SUNNYFACE];

    auto actions = ctx.constraints["action"].getAll(EQUALS);
    if (actions.size() > 1) {
      LOG(ERROR) << "You can only perform 1 action a day.";
    } else if (actions.size() == 0) {
      // Just show
      r["action"] = A_SHOW;
    } else {
      r["action"] = *actions.begin();
    }

    // A lot of things try to get destinations.
    auto dsts = getDstPositions(ctx.constraints["dst"]);
    auto srcs = ctx.constraints["src"].getAll<long long>(EQUALS);

    // Move from one tile to another.
    if (r["action"] == A_MOVE) {
      if (srcs.size() != 1) {
        LOG(ERROR) << "Invalid move. The src column must contain one value.";
      } else {
        auto src = *srcs.begin();
        r["src"] = std::to_string(src);
        if (dsts.size() != 1) {
          LOG(ERROR) << "Invalid move. The dst column must contain one value.";
        } else {
          r["dst"] = std::to_string(*dsts.begin());
          if (!tryValidMove(src, *dsts.begin())) {
            LOG(ERROR) << "Invalid move. You are trying something that is not allowed.";
          }
        }
      }
    } else if (r["action"] == A_PLANT) {
      if (srcs.size() != 0) {
        LOG(ERROR) << "Invalid plant. The src column is not used.";
      } else {
        auto single_dst = tryPlant(dsts);
        if (single_dst == 0) {
          LOG(ERROR) << "Invalid plant. You are trying something that is not allowed.";
        } else {
          r["dst"] = std::to_string(single_dst);
        }
      }
    } else if (r["action"] == A_WATER) {
      if (srcs.size() != 0) {
        LOG(ERROR) << "Invalid water. The src column is not used.";
      } else {
        auto single_dst = tryWater(dsts);
        if (single_dst == 0) {
          LOG(ERROR) << "Invalid water. You are trying something that is not allowed.";
        } else {
          r["dst"] = std::to_string(single_dst);
        }
      }
    } else if (r["action"] == A_PICKUP) {
      if (dsts.size() != 0) {
        LOG(ERROR) << "Invalid pickup. The dst column is not used.";
      } else if (srcs.size() != 1) {
        LOG(ERROR) << "Invalid pickup. The src column must contain one value.";
      } else {
        auto single_src = *srcs.begin();
        if (!tryPickup(single_src)) {
          LOG(ERROR) << "Invalid pickup. You are trying something that is not allowed.";
        } else {
          r["src"] = std::to_string(single_src);
        }
      }
    }

    if (kSheepGood && kHavePlant) {
      LOG(ERROR) << "You completed all quests. Congrats! Your prize is " << getenv("KEY");
    }

    r["farm"] = showFarm();
    if (!kSheepGood && kThreads.size() == 0) {
      kThreads.emplace_back(std::async(std::launch::async, sheepRunsAway));
    }

    return {r};
  }
};

REGISTER_EXTERNAL(FarmActionsTable, "table", "farm_actions");
REGISTER_EXTERNAL(FarmTable, "table", "farm");
REGISTER_EXTERNAL(FarmMailboxTable, "table", "farm_quests");
REGISTER_EXTERNAL(FarmEmojiTable, "table", "farm_emoji");

int main(int argc, char* argv[]) {
  osquery::Initializer runner(argc, argv, ToolType::EXTENSION);

  // bug 3: open two!
  srand (time(NULL));
  size_t sunflower = 218;
  kCells.push_back({SUNFLOWER, sunflower});

  size_t sheep;
  do {
    sheep = rand() % 255;
  } while (getX(sheep) == 0 || sheep == sunflower);
  kCells.push_back({SHEEP, sheep});

  size_t pig;
  do {
    pig = rand() % 255;
  } while (nextTo(pig, sheep) || getX(pig) == 0 || pig == sunflower);
  kCells.push_back({PIG, pig});

  size_t tractor;
  do {
    tractor = (rand() % 15) << 4;
  } while (getX(tractor) == getX(sheep) || getX(tractor) == getX(pig) || getX(tractor) == getX(sunflower));
  kCells.push_back({TRACTOR, tractor});

  for (size_t y = 1; y < 0xF; y++) {
    kCells.push_back({PLOWED, y + tractor});
  }

  LOG(WARNING) << "Welcome to the osquery farm simulator extension. You have 5 days to make your farm successful.";

  auto status = startExtension("farm", "0.0.1");
  if (!status.ok()) {
    LOG(ERROR) << status.getMessage();
    runner.requestShutdown(status.getCode());
  }

  runner.waitForShutdown();
  return 0;
}
