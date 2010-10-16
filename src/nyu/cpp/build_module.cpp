#include <nyu/cpp/build_module.hpp>
#include <nyu/cpp/config.hpp>
#include <nyu/error/dep_cycle.hpp>

#include <chilon/print_join.hpp>

namespace nyu { namespace cpp {

void build_module::subnamespace(class_type& clas) {
    if (clas.second.status_ == grammar::Status::PROCESSED) return;
    else if (clas.second.status_ == grammar::Status::PROCESSING)
        throw error::dep_cycle(clas.first);
}

void build_module::subnamespace(grammar_type& gram) {
    if (gram.second.status_ == grammar::Status::PROCESSED) return;
    else if (gram.second.status_ == grammar::Status::PROCESSING)
        throw error::dep_cycle(gram.first);

    try {
        auto extends = std::get<0>(gram.second.value_);
        if (! extends.empty()) {
            gram.second.status_ = grammar::Status::PROCESSING;
            grammar_dep(module_, extends.at<0>());
        }
    }
    catch (error::dep_cycle& err) {
        // TODO: signify is grammar, add namespace also
        err.dependencies_.push_back(gram.first);
        throw err;
    }

    // mega todo: process grammar here

    gram.second.status_ = grammar::Status::PROCESSED;
}

void build_module::operator()(enum_type& enm) {
    if (! is_open()) open();

    body_ << "\nenum class " << enm.first << " {\n" NYU_CPP_INDENT;
    if (! enm.second.value_.empty()) {
        auto it = enm.second.value_.begin();
        body_ << std::get<0>(*it);

        // todo: handle =
        for (++it; it != enm.second.value_.end(); ++it)
            body_ << ",\n" NYU_CPP_INDENT << std::get<0>(*it);
    }
    body_ << "\n};\n";
}

void build_module::close() {
    if (! stream_.is_open()) return;

    if (! module_.first.empty()) {
        auto const& module_id = module_.first.at<0>();
        open_namespace(module_id);
        stream_ << body_.str();
        close_namespace(module_id.size());
    }
    else stream_ << body_.str();

    chilon::print(stream_, "\n#endif");
    stream_ << std::flush;
    stream_.close();
}

void build_module::grammar_dep(module_type                const& module,
                               std::vector<chilon::range> const& id)
{
    // mega todo: search in current module first
    builder_.grammar_dep(module, id);
}

void build_module::open() {
    if (module_.first.empty())
        output_file::open("global.hpp", "global");
    else
        output_file::open(module_.first.at<0>());
}

} }