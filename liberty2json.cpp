#include <iostream>
#include "liberty_parser.hpp"
#include "backward.hpp"
#include "argparse.hpp"

// Main method
int main(int argc, char *argv[]) {
	// Setup signal handling
	backward::SignalHandling sh;

	// Parse arguments
  argparse::ArgumentParser program("liberty2json");
  program.add_argument("filename").help("name of the Liberty file to parse");
	program.add_argument("--outfile").help("name of the output JSON file");
	program.add_argument("--check").help("check the Liberty file for errors").flag();
	program.add_argument("--debug").help("enable debug mode").flag();
	program.add_argument("--ignore-complex-attrs").help("ignore complex attributes").flag();
  try {
    program.parse_args(argc, argv);
  }
  catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

	// Parse Liberty file
	try {
		if (program.get<bool>("--ignore-complex-attrs")) {
			LibertyParser::set_ignore_complex_attrs(true);
		}
		auto parser = new LibertyParser(program.get<std::string>("filename"), program.get<bool>("--debug"));
		if (program.get<bool>("--check")) {
			parser->check();
		}
		if (program.is_used("--outfile")) {
			parser->to_json_file(program.get<std::string>("--outfile"));
		} else {
			std::cout << parser->as_json().dump(2) << std::endl;
		}
	} catch (std::exception &e) {
		std::cerr << "FATAL: " << e.what() << std::endl;
		return 1;
	}
}
