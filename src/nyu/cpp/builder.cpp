#include <nyu/cpp/builder.hpp>
#include <nyu/cpp/module.hpp>
#include <nyu/error/not_found.hpp>
#include <nyu/error/grammar_not_found.hpp>

#include <cstring>
#include <stdexcept>

namespace nyu { namespace cpp {

void builder::generate_code() {
    // as each new depdendency is parsed.. its contents are added onto
    // the end of the include vector.. and the safe iterator will carry
    // on until all of the includes have recursively completed.
    for (auto it = chilon::make_safe_iterator(std::get<0>(ast_));
         ! it.at_end(); ++it)
    {
        std::string depFile = options_.include(*it);
        parse_file(depFile);
    }

    // parse the modules in the order they were encountered
    for (auto it = std::get<1>(ast_).safe_ordered_begin(); ! it.at_end(); ++it) {
        (*this)(*it);
    }
}

bool builder::parse_file(std::string const& file_path) {
    auto& file = add_file(file_path);

    if (file.parse_succeeded()) return false;

    options_.verbose(file_path, ": parsing");

    char const *fileData = file_path.c_str();
    if (file.parse(fileData, ast_)) {
        if (! file.parse_succeeded()) {
            file.print_parse_error(std::cout, file_path);
            throw error::parsing(file_path);
        }
        else
            options_.verbose(file_path, ": parsed");
    }
    else throw error::parsing("nothing parsed", file_path);

    return true;
}

void builder::operator()(module_type& module) {
    // auto& moduleId = module.first;
    auto& grammar = module.second.value_;

    cpp::module module_builder(*this, module);
    for (auto it = grammar.safe_ordered_begin(); ! it.at_end(); ++it)
        chilon::variant_apply(*it, module_builder);

    module_builder.close();
}

void builder::grammar_dep(module_type const& module, grammar_identifier const& id) {
    if (1 == id.size()) {
        // search in current grammar
        auto it = module.second.value_.find(*id.begin());
        if (it != module.second.value_.end()) {
            // TODO: build module
            return;
        }
        else {
            // TODO: search in parent modules
        }
    }
    else {
        // TODO: search for grammar at beginning
    }

    throw error::grammar_not_found(id);
}

void builder::print_ast() const {
    chilon::println("include ", std::get<0>(ast_));
    for (auto it = std::get<1>(ast_).begin();
         it != std::get<1>(ast_).end(); ++it)
    {
        chilon::println("module id ", it->first);
        chilon::println("module ", it->second.value_);
    }
}

} }
