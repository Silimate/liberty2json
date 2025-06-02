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
#include "json.hpp"
using json = nlohmann::json;

#ifndef LIBERTY_PARSER
#define LIBERTY_PARSER

// Liberty Parser Interface class 
class LibertyParser {
	public:
		virtual ~LibertyParser() {}; 
		virtual int check() = 0;
		virtual std::string get_error_text() = 0;
		virtual json as_json() = 0;
		virtual void to_json_file(std::string filename, bool indent) = 0;
	private:
};

#endif
