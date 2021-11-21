#include "socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>

using namespace std;

void get_URL(const string &host, const string &path) {
    // Your code here.

    // TCPSocket sock;
    // sock.connect(Address(host,"http"));
    // sock.write("GET " +path+ " HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
    // sock.shutdown(SHUT_WR);
    // while (!sock.eof())
    // {
    //     cout << sock.read();
    // }
    // sock.close();
    // return;

    // cerr << "Function called: get_URL(" << host << ", " << path << ").\n";
    // cerr << "Warning: get_URL() has not been implemented yet.\n";

    TCPSocket socket;
    socket.connect(Address(host, "http"));
    socket.write("GET " + path + " HTTP/1.1\r\n");
    socket.write("Host: " + host + "\r\n");
    socket.write("Connection: close \r\n");
    socket.write("\r\n");

    while (!socket.eof())
    {
        cout << socket.read();
    }
    socket.close();
}

int main(int argc, char *argv[]) {
    try {
        if (argc <= 0) {
            abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
        }

        // The program takes two command-line arguments: the hostname and "path" part of the URL.
        // Print the usage message unless there are these two arguments (plus the program name
        // itself, so arg count = 3 in total).
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " HOST PATH\n";
            cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
            return EXIT_FAILURE;
        }

        // Get the command-line arguments.
        const string host = argv[1];
        const string path = argv[2];

        // Call the student-written function.
        get_URL(host, path);
    } catch (const exception &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
