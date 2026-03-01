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
#pragma once

#include "LibertyParser.hh"
#include "Report.hh"

#include <fstream>
#include <string>
#include <optional>
#include <filesystem>

using string = std::string;

class Visitor;

// C++ wrapper for OpenSTA Liberty parser
class STALibertyTranslator: public sta::Report {
		Visitor *visitor_;
		std::filesystem::path filename_;
		std::optional<string> err_ = std::nullopt;
	public:
		STALibertyTranslator(string filename, std::ostream* out_stream, bool include_src_attributes = false);
		
		// LibertyParser
		~STALibertyTranslator() override;
		int check();
		string get_error_text();
		
	protected:
		// sta::Report
		virtual size_t printConsole(const char *buffer, size_t length) override;
};
