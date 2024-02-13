// Copyright 2023 Pedro Nunes
//
// This file is part of Genesis.
//
// Genesis is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Genesis is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Genesis. If not, see <http://www.gnu.org/licenses/>.

#include "crashhandler/crashhandler.h"

#include <filesystem>

#include <client/crash_report_database.h>
#include <client/crashpad_client.h>
#include <client/crashpad_info.h>
#include <client/settings.h>

namespace Genesis
{

CrashHandler::CrashHandler()
{
}

CrashHandler::~CrashHandler()
{
}

bool CrashHandler::Initialize()
{
    std::map<std::string, std::string> annotations;
    std::vector<std::string> arguments;
    std::vector<base::FilePath> attachments;

    // THE FOLLOWING ANNOTATIONS MUST BE SET.
    // Backtrace supports many file formats. Set format to minidump
    // so it knows how to process the incoming dump.
    annotations[ "format" ] = "minidump";
    annotations[ "token" ] = "92bb0906bc40eb1879e6089be088c389d00b8d8b71c49bfee43a3d6d79f28a4e";

    attachments.push_back( base::FilePath( std::filesystem::path( "log.txt=log.txt" ) ) );

    /*
     * REMOVE THIS FOR ACTUAL BUILD.
     *
     * We disable crashpad rate limiting for this example.
     */
    arguments.push_back( "--no-rate-limit" );

#if _WIN32
    base::FilePath handler( std::filesystem::path( "crashhandler/crashpad_handler.exe" ) );
#else
    base::FilePath handler( std::filesystem::path( "crashhandler/crashpad_handler" ) );
#endif

    base::FilePath db( std::filesystem::path( "crashhandler" ) );
    m_pDatabase = crashpad::CrashReportDatabase::Initialize( db );

    if ( m_pDatabase == nullptr || m_pDatabase->GetSettings() == nullptr )
    {
        return false;
    }

    // Enable automated uploads.
    m_pDatabase->GetSettings()->SetUploadsEnabled( true );

    const std::string url( "https://solace10.sp.backtrace.io:6098" );
    return crashpad::CrashpadClient{}.StartHandler( handler, db, db, url, annotations, arguments, false, false, attachments );
}

} // namespace Genesis
