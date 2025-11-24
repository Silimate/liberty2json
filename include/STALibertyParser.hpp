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

#include <fstream>
#include <string>
#include <optional>

#include "json.hpp"
#include "LibertyParser.hpp"
#include "Report.hh"
using string = std::string;
using json = nlohmann::json;

class Visitor;

// C++ wrapper for OpenSTA Liberty parser
class STALibertyParser : public LibertyParser, public sta::Report {
		Visitor *visitor_;
		std::filesystem::path filename_;
		std::optional<std::string> err_ = std::nullopt;
	public:
		STALibertyParser(string filename, bool include_src_attributes = false);
		
		// LibertyParser
		~STALibertyParser() override;
		virtual int check() override;
		virtual std::string get_error_text() override;
		virtual json as_json() override;
		virtual void to_json_file(std::string filename, bool indent) override;
		
	protected:
		// sta::Report
		virtual size_t printConsole(const char *buffer, size_t length) override;
};
