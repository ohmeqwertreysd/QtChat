#ifndef JSONCOMMANDS_H
#define JSONCOMMANDS_H
enum class Command {
    Registration,
    Login,
    Message,
    File,
    ErrorNameUsed,
    LoginFailed,
    SuccsConnect,
    ListOfOnlineUsers,
    ListOfFiles,
    FileAccepted,
    ServerNewFile,
    RequestFile
};
#endif // JSONCOMMANDS_H
