#include "fileinfo.hpp"
#include <sstream>

#define MIME_DB "/usr/share/file/magic.mgc"

namespace fileinfo
{

// Copied from https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
static int filetypeletter(int mode)
{
    char    c;

    if (S_ISREG(mode))
        c = '-';
    else if (S_ISDIR(mode))
        c = 'd';
    else if (S_ISBLK(mode))
        c = 'b';
    else if (S_ISCHR(mode))
        c = 'c';
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode))
        c = 'p';
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
    else if (S_ISLNK(mode))
        c = 'l';
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode))
        c = 's';
#endif  /* S_ISSOCK */
#ifdef S_ISDOOR
    /* Solaris 2.6, etc. */
    else if (S_ISDOOR(mode))
        c = 'D';
#endif  /* S_ISDOOR */
    else
    {
        /* Unknown type -- possibly a regular file? */
        c = '?';
    }
    return(c);
}

// Copied from https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
static char *lsperms(int mode)
{
    static const char *rwx[] = {"---", "--x", "-w-", "-wx",
    "r--", "r-x", "rw-", "rwx"};
    static char bits[11];

    bits[0] = filetypeletter(mode);
    strcpy(&bits[1], rwx[(mode >> 6)& 7]);
    strcpy(&bits[4], rwx[(mode >> 3)& 7]);
    strcpy(&bits[7], rwx[(mode & 7)]);
    if (mode & S_ISUID)
        bits[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        bits[6] = (mode & S_IXGRP) ? 's' : 'l';
    if (mode & S_ISVTX)
        bits[9] = (mode & S_IXOTH) ? 't' : 'T';
    bits[10] = '\0';
    return(bits);
}

// Helper function to easily format date/time.
std::string get_time_as_string(time_t time)
{
    char fmt_time[32];

    std::strftime(
        fmt_time,
        32,
        "%a %d.%m.%Y %H:%M:%S",
        std::localtime(&time)
    );

    return std::string(fmt_time);
}

lib::lib() noexcept
{
    this->magic_cookie = magic_open(MAGIC_MIME_TYPE);
    if (!magic_cookie)
    {
        printf("Could not create magic cookie.\n");
        exit(1);
    }
    magic_load(this->magic_cookie, MIME_DB);
}

lib::~lib() noexcept
{
    magic_close(this->magic_cookie);

}

// Returns a list of FileInfo structs as read from the lstat function.
statList lib::get_path_info(std::string path)
{
    statList files;
    struct stat curr = {};
    if (lstat(path.c_str(), &(curr)) > -1)
    {
        if (S_ISDIR(curr.st_mode))
        {
            DIR *dir_p = opendir(path.c_str());
            dirent *entry = nullptr;
            if (dir_p)
                while ((entry = readdir(dir_p)))
                {
                    struct stat curr_file = {};
                    std::string curr_path = path + "/" + (entry->d_name);
                    if (lstat(curr_path.c_str(), &curr_file) > -1)
                    {
                        files.push_back(
                            FileInfo(
                                entry->d_name,
                                magic_file(this->magic_cookie, curr_path.c_str()),
                                getpwuid(curr_file.st_uid)->pw_name,
                                getgrgid(curr_file.st_gid)->gr_name,
                                curr_file.st_atim,
                                curr_file.st_size,
                                curr_file.st_nlink,
                                curr_file.st_mode
                            )
                        );
                    }
                }
            closedir(dir_p);
        }
        else
        {
            struct stat curr_file = {};
            if (lstat(path.c_str(), &curr_file) > -1)
            {
                std::string n_path(path);
                size_t i = path.rfind('/', path.length());
                if (i != std::string::npos) {
                    path = path.substr(i+1, path.length() - i);
                }
                files.push_back(
                    FileInfo(
                        path.c_str(),
                        magic_file(
                            this->magic_cookie,
                            n_path.c_str()
                        ),
                        getpwuid(curr_file.st_uid)->pw_name,
                        getgrgid(curr_file.st_gid)->gr_name,
                        curr_file.st_atim,
                        curr_file.st_size,
                        curr_file.st_nlink,
                        curr_file.st_mode
                    )
                );
            }
        }
    }
    else if(errno == ENOENT) {
        files.push_back(
            FileInfo(
                "",
                "",
                "",
                "",
                {},
                0,
                0,
                0
            )
        );
        return files;
    }
    return files;
}

// Returns a list of FileInfo structs sorted according to the criterion specified by mode.
statList lib::get_sorted_path_info(std::string path, sortMode mode)
{
    statList files = this -> get_path_info(path);

    switch (mode)
    {
        case sortMode::ATIME:
            files.sort(
                [](FileInfo left, FileInfo right) -> bool {
                    return left.atim.tv_nsec > right.atim.tv_nsec;
                }
            );
            break;
        case sortMode::NAME:
            files.sort(
                [](FileInfo left, FileInfo right) -> bool {
                    return std::string(left.name).compare(right.name) < 0;
                }
            );
            break;
        case sortMode::SIZE:
            files.sort(
                [](FileInfo left, FileInfo right) -> bool {
                    return left.size > right.size;
                }
            );
            break;
        default:
            break;
    }

    return files;
}

// ostream overload to easily display the list of files.
std::ostream &operator << (std::ostream &out, const statList &fi)
{
    int name_l = 0, size_l = 0, uid_l = 0, gid_l = 0, time_l = 0, mime_l = 0, link_l = 0;

    for(const auto &i : fi) {
        link_l = [&i, &link_l]()->int
        {
            std::stringstream s;
            s << i.n_links;
            int len = s.str().length();
            return len > link_l ? len : link_l;
        }();
        size_l = [&i, &size_l]()->int
        {
            std::stringstream s;
            s << i.size;
            auto len = s.str().length();
            return len > size_l ? len : size_l;
        }();
        time_l = [&i, &time_l]()->int
        {
            std::string s = get_time_as_string(i.atim.tv_sec);
            return s.length() > time_l ? s.length() : time_l;
        }();
        if(i.name.length() > name_l) name_l = i.name.length();
        if(i.mime_type.length() > mime_l) mime_l = i.mime_type.length();
        if(i.user.length() > uid_l) uid_l = i.user.length();
        if(i.group.length() > gid_l) gid_l = i.group.length();
    }
    std::stringstream ss;
    for(const auto &i : fi)
    {
    ss << lsperms(i.perms)  << ' '
        << std::setw(link_l) << i.n_links << ' '
        << std::setw(uid_l)  << i.user << ' '
        << std::setw(gid_l)  << i.group << ' '
        << std::setw(size_l) << i.size << ' '
        << std::setw(time_l) << get_time_as_string(i.atim.tv_sec) << ' '
        << std::setw(name_l) << i.name << ' '
        << std::setw(mime_l) << i.mime_type << '\n';
    }
    auto x = ss.str();
    return out << x;
}

} // namespace fileinfo
