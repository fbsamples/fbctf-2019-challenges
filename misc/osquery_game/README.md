# osquery_game

We like osquery, emojis, and farm-related video games.

What if we combined them!? Complete all of the quests and win!

```
ssh osquerygame@ec2-34-235-123-28.compute-1.amazonaws.com
password: osquerygame
```

# For challenge developers

To run the challenge using Docker:

```
docker build -t osquery_game .
docker run -p 22:22 -it osquery_game
```

Install the local osqueryi version (this version silences some warnings, removes a lot of tables, and controls the printing mode).

```
sudo cp ./osqueryi /bin/osquerygame
```

Install the extension

```
sudo mkdir -p /usr/lib/osquery/extensions
sudo cp external_extension_fbctf2019.ext /usr/lib/osquery/extensions/
```

Configure the extension to load automatically

```
sudo mkdir -p /etc/osquery
echo "/usr/lib/osquery/extensions/external_extension_fbctf2019.ext" | sudo tee /etc/osquery/extensions.load
```

Add a user for people to login as

```
$ useradd osquerygame
$ passwd osquerygame
# osquerygame osquerygame
```

Verify /etc/ssh/sshd_config

```
PrintlastLog no
PrintMotd no    
PasswordAuthentication yes # aws has this as no
```

Change the `$HOME` to `/home/osquery` and `$SHELL` to `/bin/osquerygame`
Add `KEY="key{the_key}"` to `/etc/environment`

Configure the shell

```
sudo touch /home/osquery/.hushlogin # do not print lastlogin
sudo mkdir -p /home/osquery/.osquery
sudo chown osquery /home/osquery/.osquery

# This next file should stay owned by root
sudo touch /home/osquery/.osquery/.history # prevent history from recording
echo "SELECT * FROM farm_quests;" | sudo tee /home/osquery/.osquery/.history
echo "SELECT * FROM farm_actions;" | sudo tee -a /home/osquery/.osquery/.history
echo "SELECT * FROM farm;" | sudo tee -a /home/osquery/.osquery/.history
echo "SELECT to_base64('testing');" | sudo tee -a /home/osquery/.osquery/.history
echo "SELECT * FROM farm WHERE action = 'move' AND src = 0x3d AND dst = 0x64;" | sudo tee -a /home/osquery/.osquery/.history
echo "SELECT * FROM farm WHERE action = 'pickup' AND src = 0x3d;" | sudo tee -a /home/osquery/.osquery/.history
```

Optional

```
echo "--disable_watchdog" | sudo tee /etc/osquery/osquery.flags.default
```

# solving and testing

The goal is to preform 6 actions in 5 days.

## Research your env

```
ssh osquerygame@ec2-34-235-123-28.compute-1.amazonaws.com
osquerygame@ec2-34-235-123-28.compute-1.amazonaws.com's password:
Using a virtual database. Need help, type '.help'
W0428 16:17:05.787194    24 challenge.cpp:607] Welcome to the osquery farm simulator extension. You have 5 days to make your farm successful.
osquery>
```

You are in an osqueryi shell. Do some research about this.
Notice there is a `farm` extension, `.tables` shows:

```
osquery> .tables
  => farm
  => farm_actions
  => farm_quests
  => osquery_extensions
  => osquery_info
  => system_info
  => uptime
```

Hopefully they look at these tables.

## Inspect the farm

Eventually you should:

```
osquery> select * from farm;
W0428 16:17:18.555104    40 challenge.cpp:483] Good morning! It is day 1/65536 🌞
farm|action|src|dst
  0 1 2 3 4 5 6 7 8 9 A B C D E F
0🌱🌱🚜🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
1🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
2🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
3🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
4🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
5🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
6🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
7🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🐑🌱🌱🌱🌱
8🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
9🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
A🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
B🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
C🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
D🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
E🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
F🌱🌱🌱🌱🌱🐷🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
|show||
```

Immediately following you will see:

```
osquery> E0428 16:20:24.310364   193 challenge.cpp:429] The sheep was not next to his friend the pig. He saw you and ran away scared.
E0428 16:20:24.310439   193 challenge.cpp:430] You failed a quest and cannot win the game. Please retry.
```

Using knowledge from:

```
osquery> select * from farm_actions;
action|description
show|Default action, shows the farm.
move [src] [dst]|Requests to move animal in SRC field to DST field.
pickup [src]|Pickup item in SRC field.
water [...dst]|Water planted herb located at DST.
plant [...dst]|Plant a herb in the plowed DST.
osquery> SELECT * FROM farm_quests;
from|message|done
Town Mayor|The sheep wants to be next to the pig. Please move him, but be careful, if he sees you he will run away in less than a second, you need to move fast.|no
Town Mayor|Please water something that you have planted. You need to pickup a pail first. The sheep was playing with the water pail, if you move him next to his friend he may give it back.|no
Town Mayor|Please pick something that you have grown. Wait a day after planting a seed and watering then pickup your plants.|no
```

## Move the sheep

You should be able to move the pig or sheep. You will need to automate this later by parsing the farm output.

```
osquery> create table t2 as select farm from farm; select * from farm where action='move' and src=(select (((k-35)/18)*16)+((k-35)%18)-2 from (select instr(farm, '🐑') k from t2)) and dst=(select (((k-35)/18)*16)+((k-35)%18)-1 from (select instr(farm, '🐷' ) k from t2));
W0317 22:42:59.876427 32098 challenge.cpp:454] Good morning! It is day 2 🐷
farm|action|src|dst
  0 1 2 3 4 5 6 7 8 9 A B C D E F
0🌱🌱🚜🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
1🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
2🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
3🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
4🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
5🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
6🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
7🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🐷🌱🌱🌱🌱🌱
8🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
9🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
A🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
B🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
C🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
D🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
E🌱🌱🏽🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
F🌱🌱🌱🌱🌱🚰🌱🌱🌱🌱🌱🌱🌱🌱🌱🌱
|move|95|167
```

Challengers can look at their history (hit up) to see recent queries.
We include a hint there about the query construction.

## Pickup the water

```
osquery> SELECT * FROM farm WHERE action = 'pickup' AND src = 0x7B;
W0428 16:25:01.148867   407 challenge.cpp:483] Good morning! It is day 3/65536 🌞
W0428 16:25:01.148924   407 challenge.cpp:553] You have a water pail in your inventory. You can now use the 'water' action to water seeds.
```

## Plant in the plowed areas

```
select * from farm where action = 'plant' and dst = BEGIN
```

Where the BEGIN and END are the coordinates of the first plot and last plot.

## Water your planted seeds.

```
select * from farm where action = 'water' ...
```

## Overflow the day count

This is the last 'ah-ha', even if you do everything above correctly you will need 1 extra day.

So create a loop

```
osquery> WITH RECURSIVE
    ...> for(i) AS (VALUES(1) UNION ALL SELECT i+1 FROM for WHERE i < 250)
    ...> select 1 from farm, for where farm.src = for.i;
```

And then perform the last action, pickup one of your plants. ;)

But wait... your plants will turn into weeds, so repeat but do not plant/water until after you overflow.

The key is then given to you.

```
osquery> SELECT * FROM farm where action = 'pickup' and src=0x1e;
W0428 17:21:43.640535   601 challenge.cpp:486] Good morning! It is day 3/256 🌞
E0428 17:21:43.640606   601 challenge.cpp:556] You completed all quests. Congrats! Your prize is {you_win_the_game_again_again}
```

# gotchas

1. People need to understand osquery a bit and using `where ...` to control the farm.
2. The farm is implemented with emoji, if a terminal does not display emoji they will be confused.
3. They need to parse the farm output to find the pig and sheep quickly. (first part of challenge)
4. Using `plant` and `water` actions and `dst` may not be intuitive.
5. Overflowing the day is not obvious, but everything is in `0xFF`, requires experimentation. (second part of challenge)
