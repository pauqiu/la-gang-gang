#include "nodeClient.h"

int main() {
    NodeClient client;
    client.sendAuthentication("admin", "1234", 0);

    return 0;
}