/*
		liberty2json

		Copyright (C) 2025 Silimate Inc.

		This program is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "STALibertyTranslator.hpp"
#include "backward.hpp"
#include "argparse.hpp"

#include "Sta.hh"

#include <scope_guard.hpp>

#include <memory>
#include <iostream>
#include <fstream>

// Main method
int main(int argc, char *argv[]) {
	// Setup signal handling
	backward::SignalHandling sh;

	// Parse arguments
	argparse::ArgumentParser program("liberty2json");
	program.add_argument("filename").help("name of the Liberty file to parse");
	program.add_argument("--outfile", "-o").help("name of the output JSON file");
	program.add_argument("--check").help("check the Liberty file for errors only and exit").flag();
	program.add_argument("--debug").help("enable debug mode").flag();
	program.add_argument("--indent").help("enable indentation in file output").flag();
	program.add_argument("--src").help("include source attribute in top-level groups").flag();

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		return 1;
	}

	// Parse Liberty file
	try {
		sta::initSta();
		sta::Sta *sta = new sta::Sta();
		sta::Sta::setSta(sta);
		sta->makeComponents();
		
		std::ostream *out_stream = nullptr;
		MAKE_SCOPE_EXIT(scope_exit) {
			delete out_stream;
		};
		if (program.is_used("--outfile")) {
			out_stream = new std::ofstream(program.get<string>("--outfile"));
		} else if (program.get<bool>("--check")) {
#if defined(_WIN32)
			out_stream = new std::ofstream("NUL");
#else
			// Assume UNIX
			out_stream = new std::ofstream("/dev/null");
#endif
		} else {
			out_stream = &std::cout;
			scope_exit.dismiss();
		}
		
		auto parser = std::make_shared<STALibertyTranslator>(
			program.get<string>("filename"),
			out_stream,
			program.get<bool>("--src")
		);
		
		if (parser->check()) {
			std::cerr << "ERROR: " << parser->get_error_text() << std::endl;
			return 1;
		}
		return 0;
	} catch (std::exception &e) {
		std::cerr << "FATAL: " << e.what() << std::endl;
		return 1;
	}
}
