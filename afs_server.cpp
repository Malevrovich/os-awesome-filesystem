#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <boost/asio.hpp>

extern "C" {
#include "afs_protocol.h"
}

using boost::asio::ip::tcp;

class FileSystem {
 public:
  FileSystem() { fs[ROOT_HANDLE] = std::make_shared<Directory>(ROOT_HANDLE, ""); }

  afs_response processRequest(const afs_request& req) {
    if (req.type == AFS_CREATE) {
      std::cout << "CREATE" << std::endl;
      auto ptr = fs[req.args.as_create.dir].get();

      if (ptr->is_dir) {
        std::cout << "DIR" << std::endl;
        auto dir_ptr = static_cast<Directory*>(ptr);

        auto new_handle = next_handle++;

        auto file_uptr = std::make_shared<DataFile>(new_handle, std::string(req.args.as_create.name));

        auto [iter, success] = fs.emplace(new_handle, file_uptr);
        dir_ptr->data.emplace_back(file_uptr);

        std::cout << "OK" << std::endl;
        return afs_response{.status = AFS_OK, .body = {.as_create = new_handle}};
      }
      return afs_response{.status = AFS_ERROR};
    }

    if (req.type == AFS_LOOKUP) {
      std::cout << "LOOKUP" << std::endl;
      auto ptr = fs[req.args.as_lookup.dir].get();

      if (ptr->is_dir) {
        std::cout << "DIR" << std::endl;
        auto dir_ptr = static_cast<Directory*>(ptr);

        std::string to_find(req.args.as_lookup.name);

        for (auto& base_ptr : dir_ptr->data) {
          if (to_find == base_ptr->name) {
            auto res = afs_response{
                .status = AFS_OK,
                .body = {.as_lookup = {.handle = base_ptr->handle, .type = base_ptr->is_dir ? AFS_DIR : AFS_FILE}}};

            strcpy(res.body.as_lookup.name, base_ptr->name.data());
            res.body.as_lookup.name[base_ptr->name.size()] = '\0';

            std::cout << "OK" << std::endl;
            return res;
          }
        }

        return afs_response{.status = AFS_ERROR};
      }
      return afs_response{.status = AFS_ERROR};
    }

    if (req.type == AFS_READDIR) {
      std::cout << "READDIR" << std::endl;
      auto ptr = fs[req.args.as_readdir.dir].get();

      if (ptr->is_dir) {
        std::cout << "DIR" << std::endl;
        auto dir_ptr = static_cast<Directory*>(ptr);

        afs_response res = {.status = AFS_OK};

        int i = 0;
        for (i = 0; i < std::min<int>(MAX_FILES_PER_READDIR, dir_ptr->data.size()); i++) {
          res.body.as_readdir[i] = {.handle = dir_ptr->data[i]->handle,
                                    .type = dir_ptr->data[i]->is_dir ? AFS_DIR : AFS_FILE};

          strcpy(res.body.as_readdir[i].name, dir_ptr->data[i]->name.data());
          res.body.as_readdir[i].name[dir_ptr->data[i]->name.size()] = '\0';
        }

        if (i < MAX_FILES_PER_READDIR) {
          res.body.as_readdir[i] = {.handle = 0};
        }

        std::cout << "OK " << i << std::endl;
        return res;
      }
      return afs_response{.status = AFS_ERROR};
    }

    if (req.type == AFS_UNLINK) {
      std::cout << "UNLINK" << std::endl;
      auto ptr = fs[req.args.as_unlink.dir].get();

      if (ptr->is_dir) {
        std::cout << "DIR" << std::endl;
        auto dir_ptr = static_cast<Directory*>(ptr);

        std::string to_find(req.args.as_unlink.name);
        int pos = 0;

        for (auto& base_ptr : dir_ptr->data) {
          if (to_find == base_ptr->name) {
            break;
          }
          pos++;
        }

        if (pos == dir_ptr->data.size()) {
          return afs_response{.status = AFS_ERROR};
        }

        dir_ptr->data.erase(dir_ptr->data.begin() + pos);
        return afs_response{.status = AFS_OK};
      }
      return afs_response{.status = AFS_ERROR};
    }

    if (req.type == AFS_MKDIR) {
      std::cout << "MKDIR" << std::endl;
      auto ptr = fs[req.args.as_create.dir].get();

      if (ptr->is_dir) {
        std::cout << "DIR" << std::endl;
        auto dir_ptr = static_cast<Directory*>(ptr);

        auto new_handle = next_handle++;

        auto file_uptr = std::make_shared<Directory>(new_handle, std::string(req.args.as_create.name));

        auto [iter, success] = fs.emplace(new_handle, file_uptr);
        dir_ptr->data.emplace_back(file_uptr);

        std::cout << "OK" << std::endl;
        return afs_response{.status = AFS_OK, .body = {.as_create = new_handle}};
      }
      return afs_response{.status = AFS_ERROR};
    }

    return afs_response{.status = AFS_ERROR};
  }

 private:
  struct BaseFile {
    BaseFile(fhandle handle, const std::string& name, bool is_dir) : handle(handle), name(name), is_dir(is_dir) {}

    fhandle handle;
    bool is_dir;
    std::string name;
  };

  struct DataFile : public BaseFile {
    DataFile(fhandle handle, const std::string& name) : BaseFile(handle, name, false) {}
    std::string data;
  };

  struct Directory : public BaseFile {
    Directory(fhandle handle, const std::string& name) : BaseFile(handle, name, true) {}
    std::vector<std::shared_ptr<BaseFile>> data;
  };

  uint next_handle = ROOT_HANDLE + 1;
  std::map<fhandle, std::shared_ptr<BaseFile>> fs;
};

int main() {
  FileSystem fs;
  try {
    boost::asio::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), PORT));

    for (;;) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      std::cout << "ACCEPTED" << std::endl;

      for (;;) {
        afs_request req;
        boost::system::error_code error;

        size_t len = socket.read_some(boost::asio::buffer(&req, sizeof(afs_request)), error);

        if (error || len != sizeof(afs_request)) {
          if (error) {
            std::cout << "disconnecting by error" << std::endl;
            std::cout << error << std::endl;
            std::cout << error.message() << std::endl;
            std::cout << "received: " << len << " expected " << sizeof(afs_request) << std::endl;
          } else {
            std::cout << "disconnecting by length: " << len << " expected: " << sizeof(afs_request) << std::endl;
          }
          break;
        }

        afs_response res = fs.processRequest(req);

        boost::asio::write(socket, boost::asio::buffer(&res, sizeof(afs_response)), error);

        if (error) {
          std::cout << "disconnecting" << std::endl;
          break;
        }
      }

      socket.close();
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}