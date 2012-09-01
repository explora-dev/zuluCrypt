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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow( QWidget * parent ) : QMainWindow( parent ),m_ui( new Ui::MainWindow )
{
	m_ui->setupUi( this );

	m_ui->lineEditKey->setEchoMode( QLineEdit::Password );

	m_zuluSocket = 0 ;

	this->setWindowIcon( QIcon( QString( ":/gpg.png" ) ) );
	m_ui->pbKeyFile->setIcon( QIcon( QString( ":/gpg.png" ) ) );

	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( pbCancel() ) ) ;
	connect( m_ui->pbOpen,SIGNAL( clicked() ),this,SLOT( pbOpen() ) ) ;
	connect( m_ui->pbKeyFile,SIGNAL( clicked() ),this,SLOT( pbKeyFile() ) ) ;

	this->SetFocus();
	m_ui->pbOpen->setEnabled( false );
}

void MainWindow::SetAddr( QString addr )
{
	m_addr = addr ;
	m_zuluSocket = new zuluSocket( this ) ;
	connect( m_zuluSocket,SIGNAL( gotConnected() ),this,SLOT( gotConnected() ) ) ;
	connect( m_zuluSocket,SIGNAL( doneWritingData() ),this,SLOT( doneWritingData() ) ) ;
	m_zuluSocket->startServer( m_addr );
}

void MainWindow::gotConnected()
{
	m_ui->pbOpen->setEnabled( true );
}

void MainWindow::SetFocus()
{
	if( m_ui->lineEditKey->text().isEmpty() )
		m_ui->lineEditKey->setFocus();
	else if( m_ui->lineEditKeyFile->text().isEmpty() )
		m_ui->lineEditKeyFile->setFocus();
	else
		m_ui->pbOpen->setFocus();
}

void MainWindow::pbCancel()
{
	this->Exit( 1 );
}

void MainWindow::Exit( int st )
{
	this->hide();
	QCoreApplication::exit( st );
}

QString MainWindow::FindGPG()
{
	if( QFile::exists( QString( "/usr/local/bin/gpg" ) ) )
		return QString( "/usr/local/bin/gpg" ) ;
	else if( QFile::exists( QString( "/usr/bin/gpg" ) ) )
		return QString( "/usr/bin/gpg" ) ;
	else if( QFile::exists( QString( "/usr/sbin/gpg") ) )
		return QString( "/usr/sbin/gpg" ) ;
	else{
		QString m ;
		return m ;
	}
}

void MainWindow::pbOpen()
{
	QString path = m_ui->lineEditKeyFile->text().replace( "\"","\"\"\"" ) ;

	DialogMsg msg( this ) ;

	if( !QFile::exists( path ) )
		return msg.ShowUIOK( tr( "ERROR" ),tr( "invalid path to gpg keyfile" ) ) ;

	QString key = m_ui->lineEditKey->text().replace( "\"","\"\"\"" ) ;

	if( key.isEmpty() )
		return msg.ShowUIOK( tr( "ERROR" ),tr( "key field is empty" ) ) ;

	this->disableAll();
	QString EXE = this->FindGPG() ;

	if( EXE.isEmpty() )
		return msg.ShowUIOK( tr( "ERROR" ),tr( "could not find \"gpg\" executable in \"/usr/local\",\"/usr/bin\" and \"/usr/sbin\"" ) ) ;

	QString exe = QString( "%1 --batch --passphrase \"%2\" -d \"%3\"" ).arg( EXE ).arg( key ).arg( path ) ;

	QProcess p ;
	p.start( exe );
	p.waitForFinished() ;

	if( p.exitCode() == 0 ){
		QByteArray data = p.readAll() ;

		if( !data.isEmpty() )
			m_zuluSocket->sendData( &data );
	}else{
		DialogMsg msg( this ) ;
		msg.ShowUIOK( tr( "ERROR" ),tr("could not decrept the gpg keyfile,wrong key?" ) );
		this->enableAlll();
	}
	p.close();
}

void MainWindow::doneWritingData()
{
	this->enableAlll();
	this->hide();
	this->Exit( 0 );
}

void MainWindow::pbKeyFile()
{
	QString Z = QFileDialog::getOpenFileName( this,QString( "select a key file" ),QDir::homePath() ) ;

	if( !Z.isEmpty() )
		m_ui->lineEditKeyFile->setText( Z );
	this->SetFocus();
}

void MainWindow::closeEvent( QCloseEvent * e )
{
	e->ignore();
	this->doneWritingData();
}

void MainWindow::disableAll()
{
	m_ui->label->setEnabled( false );
	m_ui->label_2->setEnabled( false );
	m_ui->lineEditKey->setEnabled( false );
	m_ui->lineEditKeyFile->setEnabled( false );
	m_ui->pbKeyFile->setEnabled( false );
	m_ui->pbOpen->setEnabled( false );
	m_ui->pbCancel->setEnabled( false );
}

void MainWindow::enableAlll()
{
	m_ui->label->setEnabled( true );
	m_ui->label_2->setEnabled( true );
	m_ui->lineEditKey->setEnabled( true );
	m_ui->lineEditKeyFile->setEnabled( true );
	m_ui->pbKeyFile->setEnabled( true );
	m_ui->pbOpen->setEnabled( true );
	m_ui->pbCancel->setEnabled( true );
}

MainWindow::~MainWindow()
{
	if( m_zuluSocket )
		m_zuluSocket->deleteLater();

	delete m_ui;
}
