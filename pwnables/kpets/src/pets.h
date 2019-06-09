
struct dog {
	unsigned int type;
    unsigned int name_len;
	char name[128];
};

struct cat {
	unsigned int type;
	char name[16];
	int age;
};

struct owner {
    char name[32];
    char location[32];
};


struct pet {
    char type;
    unsigned int name_len;
    char name[32];
    unsigned int description_len;
    char description[64];
};
