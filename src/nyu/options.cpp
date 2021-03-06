#include <nyu/options.hpp>
#include <nyu/error/output_file.hpp>
#include <nyu/config.hpp>

#include <chilon/conf/cmd/command_line.hpp>
#include <chilon/parser/joined.hpp>
#include <chilon/parser/char_range.hpp>
#include <chilon/parser/char.hpp>
#include <chilon/parser/many.hpp>
#include <chilon/filesystem/mkpath.hpp>
#include <chilon/print_join.hpp>

#include <sstream>

namespace nyu {

#define NYU_DEFAULT_NAMESPACE_ALIAS "chpar"

namespace cmd_line = chilon::conf::cmd;

options::options()
  : verbose_(false), verbose_comments_(false),
    print_ast_(false), output_dir_("."),
    namespace_alias_(NYU_DEFAULT_NAMESPACE_ALIAS) {}

int options::parse_command_line(char const *header, int argc, char *argv[]) {
    int nPositionals;

    using chilon::conf::value;
    using cmd_line::options_description;

    options_description opt_parser;
    opt_parser.add_options()
        .help(header)
        ("p,print",      print_ast_, "print AST of grammar")
        ("v,verbose",    verbose_, "increase verbosity")
        ("o,output-dir", output_dir_, "directory to output code")
        ("n,namespace",  namespace_alias_,
         "chilon parser namespace alias, default: " NYU_DEFAULT_NAMESPACE_ALIAS)
        ("V",            verbose_comments_, "print lots of comments to generated parser")
        ("I,include",    include_paths_, "include paths")
        ;

    try {
        nPositionals = cmd_line::parser(argc, argv, opt_parser)(std::cerr).n_positionals();
    }
    catch (cmd_line::invalid_arguments& ) {
        return 0;
    }
    catch (cmd_line::bad_value& e) {
        std::cerr << "bad value reading command line options\n";
        std::cout << opt_parser << std::endl;
        return 0;
    }
    catch (cmd_line::expected_argument& e) {
        std::cerr << "expected command line argument\n";
        std::cout << opt_parser << std::endl;
        return 0;
    }

    include_paths_.push_back(std::string(NYU_SYSTEM_INCLUDE_PATH));

    if (nPositionals < 1) {
        std::cerr << "please supply at least one grammar to parse\n";
        std::cout << opt_parser << std::endl;
        return 0;
    }

    return nPositionals;
}

std::string options::include(std::vector<chilon::range> const& path) const {
    std::stringstream s;
    s << '/';
    chilon::print_join(s, '/', path);
    s << ".nyah";
    for (auto it = include_paths_.begin(); it != include_paths_.end();
         ++it)
    {
        std::string search = *it + s.str();
        if (! access(search.c_str(), R_OK)) return search;
    }

    return "";
}

void options::output_path(std::ofstream&     stream,
                          std::string const& filename) const
{
    output_path_helper(stream, output_dir_ + "/" + filename);
}

void options::output_path(
    std::ofstream&                    stream,
    std::vector<chilon::range> const& path,
    char const * const                suffix) const
{
    std::stringstream s;
    chilon::print_join(s, '/', output_dir_, path);
    s << suffix;
    output_path_helper(stream, s.str());
}

void options::output_path(
    std::ofstream&                    stream,
    std::vector<chilon::range> const& path,
    chilon::range const&              id,
    char const * const                suffix) const
{
    std::stringstream s;
    chilon::print_join(s, '/', output_dir_, path);
    s << '/' << id << suffix;
    output_path_helper(stream, s.str());
}

void options::output_path_helper(std::ofstream&     stream,
                                 std::string const& filename) const
{
    if (! chilon::filesystem::mkpath_containing(filename))
        throw error::cannot_open_output_file(filename);

    stream.open(filename);
    if (! stream.good())
        throw error::cannot_open_output_file(filename);
}

}
