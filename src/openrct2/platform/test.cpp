//
//  test.cpp
//  OpenRCT2
//
//  Created by Lewis Fox on 2/25/17.
//  Copyright © 2017 OpenRCT2. All rights reserved.
//

#include <boost/filesystem.hpp>

extern "C" {
#include "../common.h"
#include "platform.h"
	
	bool platform_directory_delete(const utf8 *path)
	{
		log_verbose("Recursively deleting directory %s", path);
		
		boost::filesystem::path boost_path(path);
		boost::filesystem::remove_all(boost_path);
		return boost::filesystem::exists(boost_path);
	}
	
}
