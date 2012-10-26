/*
 *
 *  Copyright ( c ) 2012
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  ( at your option ) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "openmountpointinfilemanager.h"

openmountpointinfilemanager::openmountpointinfilemanager( QString path )
{
	m_path = path ;
}

void openmountpointinfilemanager::start()
{
	QThreadPool::globalInstance()->start( this );
}

void openmountpointinfilemanager::run()
{
	QDir dir( m_path ) ;
	if( dir.exists() ){
		QProcess exe ;
		exe.start( QString( "xdg-open " ) + m_path );
		exe.waitForFinished() ;
		exe.close();
	}
}

openmountpointinfilemanager::~openmountpointinfilemanager()
{
}