INC=/usr/local/ssl/include/
LIB=/usr/local/ssl/lib/
all:
	gcc -I$(INC) -L$(LIB) -o km KM.c -lcrypto -ldl
	gcc -I$(INC) -L$(LIB) -o a A.c -lcrypto -ldl
	gcc -I$(INC) -L$(LIB) -o b B.c -lcrypto -ldl
