
#define SAFE_DEL(p) \
	do{\
		if ((p) != NULL){\
			delete (p);\
			(p) = NULL;\
		}\
	}while(0)

#define SAFE_CLOSEDEL(p)\
	do{\
		if ((p) != NULL){\
			(p)->Close();\
			delete (p);\
			(p) = NULL;\
		}\
	}while(0)