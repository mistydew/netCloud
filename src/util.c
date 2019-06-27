// Copyright (c) 2017-2019 mistydew
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"

void LicenseInfo()
{
    char versionInfo[] = {"netCloud Core Daemon version v0.0.1.0\n"};
    fprintf(stdout, "%s", versionInfo);
}

void HelpMessage()
{
    char strUsage[] = {"Usage:\n\n"
                       "  ftpd [options]       Start netCloud Core Daemon\n\n"
                       "Options:\n\n"
                       "  -? / -h / -help\n"
                       "       This help message\n\n"
                       "  -version\n"
                       "       Print version and exit\n\n"
                       "  -conf=<file>\n"
                       "       Specify configuration file (default: netcloud.conf)\n\n"
                       "  -daemon\n"
                       "       Run in the background as a daemon and accept commands\n\n"
                       "  -datadir=<dir>\n"
                       "       Specify data directory\n\n"
                       "Connection options:\n\n"
                       "  -bind=<addr>\n"
                       "       Bind to given address and always listen to it (default: 127.0.0.1)\n\n"
                       "  -listen\n"
                       "       Accept connections from outside (default: 2)\n\n"
                       "  -port=<port>\n"
                       "       Listen for connections on <port> (default: 8888)\n\n"
                       "  -threads\n"
                       "       Set the number of threads to service RPC calls (default: 2)\n"};
    fprintf(stdout, "%s", strUsage);
}

void ParseParameters(int argc, char* argv[])
{
}

void GetDefaultDataDir(char* path)
{
    // Unix: ~/.netCloud
    char* pszHome = getenv("HOME");
    if (pszHome == NULL || strlen(pszHome) == 0)
        strcpy(path, "/");
    else
        sprintf(path, "%s%s", pszHome, "/");

    sprintf(path, "%s%s", path, ".netCloud");
}

void GetDataDir(char* path)
{
    GetDefaultDataDir(path);
    mkdir(path, 0775);
}

void GetConfigFile(char* path)
{
    sprintf(path, "%s%s%s", path, "/", "netCloud.conf");
}

void ReadConfigFile(char* pathConfigFile)
{
    int fdConfig = open(pathConfigFile, O_RDONLY);

    char buf[1024] = {0};
    read(fdConfig, buf, sizeof(char) * 1024);

    char ** argvConf = (char**)calloc(3, sizeof(char*));
    for (int idx = 0; idx != 3; ++idx)
    {
        argvConf[idx] = (char*)calloc(1, sizeof(char) * 16);
    }

    for (int idx = 0, y =0; idx != strlen(buf); ++idx)
    {
        if (buf[idx] == '=')
        {
            ++idx;
            for (int x = 0; buf[idx] != '\n'; ++idx, ++x)
            {
                argvConf[y][x] = buf[idx];
            }
            ++y;
        }
    }

    close(fdConfig);
}

void sigHandler(int signum)
{
    char flag = 1;
    write(exitfd[1], &flag, sizeof(char));
}

void setExit()
{
    pipe(exitfd);
    if (fork()) {
        close(exitfd[0]);
        signal(SIGINT, sigHandler);
        wait(NULL);
        exit(0);
    }
    close(exitfd[1]);
    setsid();
    signal(SIGPIPE, SIG_IGN);
}

int InitSocket()
{
    // Socket
    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    // Set socket port reuse
    int reuse = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuse, sizeof(reuse));

    // Bind
    struct sockaddr_in ser;
    bzero(&ser, sizeof(struct sockaddr_in));
    ser.sin_family = AF_INET;
    ser.sin_port = htons(mapArgs.nPort);
    ser.sin_addr.s_addr = inet_addr(mapArgs.sIP);
    bind(sfd, (struct sockaddr*)&ser, sizeof(ser));

    // Listen
    listen(sfd, 20);

    return sfd;
}
