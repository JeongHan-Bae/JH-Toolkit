# <jh/ipc> forwards to the IPC implementation. The implementation itself is
# the synchronous aggregate header and its implementation directory.
set(JH_TOOLKIT_IPC_FORWARD_AGGREGATE "jh/ipc")
set(JH_TOOLKIT_IPC_IMPLEMENTATION_HEADER "jh/synchronous/ipc.h")
set(JH_TOOLKIT_IPC_IMPLEMENTATION_DIRECTORY "jh/synchronous/ipc")
set(JH_TOOLKIT_IPC_OWNED_PATHS
    "${JH_TOOLKIT_IPC_FORWARD_AGGREGATE}"
    "${JH_TOOLKIT_IPC_IMPLEMENTATION_HEADER}"
    "${JH_TOOLKIT_IPC_IMPLEMENTATION_DIRECTORY}")
