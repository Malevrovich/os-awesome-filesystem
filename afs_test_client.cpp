#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <boost/asio.hpp>

extern "C" {
#include "afs_protocol.h"
}

using boost::asio::ip::tcp;

afs_dir_op_args parseDirOpArgs() {
  afs_dir_op_args args = {};

  fhandle handle;
  std::string name;

  std::cin >> handle;
  args.dir = handle;

  std::cin >> name;

  strcpy(args.name, name.data());
  args.name[name.size()] = '\0';

  return args;
}

void printLookupResult(std::ostream& out, afs_lookup_result res) {
  out << "Handle: " << res.handle << std::endl;
  out << "Type: " << ((res.type == AFS_DIR) ? "DIR" : "FILE") << std::endl;
  out << "Name: " << res.name << std::endl;
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints = resolver.resolve(argv[1], "daytime");

    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    for (;;) {
      afs_request req = {};

      std::string inp;
      std::cin >> inp;
      if (inp == "CREATE") {
        req.type = AFS_CREATE;
        req.args.as_create = parseDirOpArgs();
      } else if (inp == "LOOKUP") {
        req.type = AFS_LOOKUP;
        req.args.as_lookup = parseDirOpArgs();
      } else if (inp == "READDIR") {
        req.type = AFS_READDIR;
        fhandle handle;
        std::cin >> handle;
        req.args.as_readdir.dir = handle;
      }

      boost::system::error_code error;
      boost::asio::write(socket, boost::asio::buffer(&req, sizeof(afs_request)), error);

      if (error) {
        std::cout << "ERROR " << error << std::endl;
        break;
      }

      afs_response res = {};

      size_t len = socket.read_some(boost::asio::buffer(&res, sizeof(afs_response)), error);

      if (res.status == AFS_OK) {
        std::cout << "Ok" << std::endl;

        if (req.type == AFS_CREATE) {
          std::cout << "Handle: " << res.body.as_create << std::endl;
        } else if (req.type == AFS_LOOKUP) {
          printLookupResult(std::cout, res.body.as_lookup);
        } else {
          for (int i = 0; i < MAX_FILES_PER_READDIR; i++) {
            if (res.body.as_readdir[i].handle == 0) {
              break;
            }
            std::cout << "Subdir " << i << ":" << std::endl;
            printLookupResult(std::cout, res.body.as_readdir[i]);
          }
        }
      } else {
        std::cout << "Error" << std::endl;
      }
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}