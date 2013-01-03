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

#define OSC_THREAD_SLEEP_FOR    16.6f

// OSC addresses
#define OSC_ADDRESS_PLAY    "/play"         // out: /play PLAY_MODE STATE           in: /play STATE(free_run default mode) | /play PLAY_MODE STATE
#define OSC_ADDRESS_CUE     "/cue"          // out: /cue START_T DURATION T_NORM    in: /cue INDEX(play cue)
//#define OSC_ADDRESS_MODULE  "/module"       // out: /play PLAY_MODE STATE           in: /play STATE(free_run default mode)

//#define OSC_ADDRESS_

class QTimelineOscController
{
    
public:
    
    QTimelineOscController( int inPort = 9000, int outPort = 9001 )
    {
        mListener   = NULL;
        mSender     = NULL;
        
        init( inPort, outPort );
    }
    
    ~QTimelineOscController()
    {
        closeConnections();
    }
    
    void init( int inPort, int outPort )
    {
        closeConnections();
        
        mListener   = new ci::osc::Listener();
        mSender     = new ci::osc::Sender();
        
        mInPort     = inPort;
        mOutPort    = outPort;
        
        std::string host = "localhost";
        
        // TODO remove this!!!
        addClient( host, mOutPort ); // test client
        
        mSender->setup( host, mOutPort );
        
        mListener->setup( mInPort );
        
        std::thread sendDataThread(     &QTimelineOscController::sendData,      this);
        std::thread receiveDataThread(  &QTimelineOscController::receiveData,   this);
    }
    
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
    
    void closeConnections()
    {
        if ( mListener )
        {
            mListener->shutdown();
            delete mListener;
            mListener = NULL;
        }
        
        if ( mSender )
        {
            delete mSender;
            mSender = NULL;
        }
    }
    
    void sendData()
    {
        while( true )
        {
            if ( mBundle.getMessageCount() > 0 )
                mSender->sendBundle( mBundle );
            
            mBundle.clear();
            
            ci::sleep( OSC_THREAD_SLEEP_FOR );
        }
    }
    
    void receiveData()
    {
        while( mListener )
        {
            while ( mListener->hasWaitingMessages() )
            {
                ci::osc::Message message;
                mListener->getNextMessage( &message );
                
                std::string address = message.getAddress();
                
                if ( true )
                    debugMessage( message );
                
//                // Raw Fft left channel
//                else if ( boost::find_first( address, "/fft/1" ) )
//                    parseRawFft(message, AUDIO_LEFT_CHANNEL);
            }
            
            ci::sleep( OSC_THREAD_SLEEP_FOR );
            // boost::this_thread::sleep(boost::posix_time::milliseconds(15));
        }
    }
    
private:
    
    void debugMessage( ci::osc::Message message )
    {
        ci::app::console() << "New message received" << std::endl;
        ci::app::console() << "Address: " << message.getAddress() << std::endl;
        ci::app::console() << "Num Arg: " << message.getNumArgs() << std::endl;
        
        for (int i = 0; i < message.getNumArgs(); i++)
        {
            ci::app::console() << "-- Argument " << i << std::endl;
            ci::app::console() << "---- type: " << message.getArgTypeName(i) << std::endl;
            
            if ( message.getArgType(i) == ci::osc::TYPE_INT32 )
            {
                try {
                    ci::app::console() << "------ value: "<< message.getArgAsInt32(i) << std::endl;
                }
                catch (...) {
                    ci::app::console() << "Exception reading argument as int32" << std::endl;
                }
                
            }
            
            else if ( message.getArgType(i) == ci::osc::TYPE_FLOAT )
            {
                try {
                    ci::app::console() << "------ value: " << message.getArgAsFloat(i) << std::endl;
                }
                catch (...) {
                    ci::app::console() << "Exception reading argument as float" << std::endl;
                }
            }
            
            else if ( message.getArgType(i) == ci::osc::TYPE_STRING )
            {
                try {
                    ci::app::console() << "------ value: " << message.getArgAsString(i).c_str() << std::endl;
                }
                catch (...) {
                    ci::app::console() << "Exception reading argument as string" << std::endl;
                }
            }
        }
    }
    
private:
    
    struct QTimelineOscClient
    {
        std::string host;
        int         port;
    };
    
    ci::osc::Sender                     *mSender;
    ci::osc::Listener                   *mListener;
    std::vector<QTimelineOscClient>     mClients;
    ci::osc::Bundle                     mBundle;
    int                                 mInPort, mOutPort;
    
};

#endif