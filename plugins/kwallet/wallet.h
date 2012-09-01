/*
 *
 *  Copyright (c) 2012
 *  name : mhogo mchungu
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WALLET_H
#define WALLET_H

#include <kwallet.h>
#include <QDebug>
#include <QObject>
#include <QCoreApplication>
#include <QMap>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QByteArray>
#include "../../zuluCrypt-gui/zuluoptions.h"
#include "wallet.h"

#include "zulusocket.h"

#include <cstdlib>

using namespace KWallet ;

class wallet : public QObject
{
	Q_OBJECT
public:
	wallet( QString uuid,QString sockAddr ) ;
	~wallet();
private slots:
	void start( void ) ;
	void openWallet( void ) ;
private:
	void SendKey( void ) ;
	void Exit( int ) ;
	void readKwallet( void ) ;
	zuluSocket * m_zuluSocket ;
	Wallet * m_wallet ;
	QString m_uuid ;
	QString m_sockAddr ;
	QLocalServer * m_server ;
	QByteArray m_key ;
	int m_status ;
};

#endif // WALLET_H
