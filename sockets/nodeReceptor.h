#ifndef NODERECEPTOR_H
#define NODERECEPTOR_H

#include "node_base.h"
#include "nodeStorage.h"
#include "filesystem"

class nodeReceptor:NodeBase {
    nodeReceptor(int port, FileSystem * fileSystem)
        : NodeBase(port), fileSystem(fileSystem) {

        // TODO(@Paulette): Register handler by message.
    }



private:
    FileSystem * fileSystem;

    // TODO(@Paulette): Implement log for this node.
};

#endif // NODERECEPTOR_H
