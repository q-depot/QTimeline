/*
 *  QTimelineOscController.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_OSC_CONTROLLER
#define QTIMELINE_OSC_CONTROLLER

#pragma once

#include "OscSender.h"
#include "OscListener.h"
#include "OscBundle.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "cinder/Thread.h"

typedef std::shared_ptr<class QTimelineOscController>    QTimelineOscControllerRef;


class QTimelineOscController
{
    
public:
    
    QTimelineOscController()
    {
        std::string host = "169.192.0.10";
        int         port = 9005;
        
        addClient( host, port ); // test client
        
        mSender.setup( host, port );
        
        std::thread sendDataThread( &QTimelineOscController::sendData, this);
    }
    
    ~QTimelineOscController() {}
    
    void addClient( std::string host, int port )
    {
        for( size_t k=0; k < mClients.size(); k++ )
            if ( mClients[k].host == host && mClients[k].port == port )
                return;
        
        QTimelineOscClient c = { host, port };
        mClients.push_back( c );
    }
    
    
    void removeClient( std::string host, int port )
    {
        for( size_t k=0; k < mClients.size(); k++ )
            if ( mClients[k].host == host && mClients[k].port == port )
            {
                mClients.erase( mClients.begin() + k );
                return;
            }
    }

    
    void addMessage( std::string address, std::string args )
    {
        if ( mClients.empty() )
            return;
        
        ci::osc::Message            message;
        std::vector<std::string>    splitValues;
        
        boost::split( splitValues, args, boost::is_any_of(" ") );
        
        message.setAddress( address );
        
        for ( int k = 0; k < splitValues.size(); k++ )
        {
            char argType = splitValues[k][0];
            splitValues[k].erase(0,1);
            
            if ( argType == 'i' )
                message.addIntArg( boost::lexical_cast<int>( splitValues[k] ) );
            
            else if ( argType == 'f' )
                message.addFloatArg( boost::lexical_cast<float>( splitValues[k] ) );
            
            else if ( argType == 's' )
                message.addStringArg( splitValues[k] );
        }
        
        for( size_t k=0; k < mClients.size(); k++ )
        {
            message.setRemoteEndpoint( mClients[k].host, mClients[k].port );
            mBundle.addMessage( message );
        }
    }
    
    
private:
    
    void sendData()
    {
        while( true )
        {
            if ( mBundle.getMessageCount() > 0 )
                mSender.sendBundle( mBundle );
            
            mBundle.clear();
            
            ci::sleep( 15.0f );
        }
    }
    
    void receiveData()
    {
        
    }
    
    
private:
    
    struct QTimelineOscClient
    {
        std::string host;
        int         port;
    };
    
    ci::osc::Sender                     mSender;
    std::vector<QTimelineOscClient>     mClients;
    ci::osc::Bundle                     mBundle;
    
    ci::osc::Listener                   mListener;
    int                                 mListenerPort;

};

#endif