#ifndef LIB_FILEINFO_HPP
#define LIB_FILEINFO_HPP

#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include <dirent.h>
#include <grp.h>
#include <magic.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fileinfo
{
enum class sortMode
{
  DEFAULT,
  NAME,
  SIZE,
  ATIME
};

struct FileInfo
{
  std::string mime_type;
  std::string name;
  std::string user;
  std::string group;
  timespec atim;
  size_t size;
  size_t n_links;
  mode_t perms;

  FileInfo(
      const char *name,
      const char *mime_type,
      const char *user,
      const char *group,
      timespec atim,
      size_t size,
      size_t n_links,
      mode_t perms) : name(name),
                      mime_type(mime_type),
                      user(user),
                      group(group),
                      atim(atim),
                      size(size),
                      n_links(n_links),
                      perms(perms) {}
};

typedef std::list<FileInfo> statList;

class lib
{
private:
  magic_t magic_cookie;

public:
  lib() noexcept;
  ~lib() noexcept;
  statList get_path_info(std::string path);
  statList get_sorted_path_info(std::string path, sortMode mode);
};

std::ostream &operator<<(std::ostream &out, const statList &fi);
} // namespace fileinfo

#endif //LIB_FILEINFO_HPP
