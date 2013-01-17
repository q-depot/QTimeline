/*
 *  BasicModule.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#pragma once

#include "QTimelineModule.h"

#include "cinder/Rand.h"
#include "cinder/Utilities.h"


class BasicModule : public QTimelineModule
{
    
public:
    
    BasicModule() : QTimelineModule( "BasicModule" )
    {
        
    }
    
    BasicModule( QTimelineItemRef item ) : QTimelineModule( item, "BasicModule" )
    {
        mColor  = ci::Color::white();
        mPos.x  = ci::Rand::randInt( ci::app::getWindowWidth() );
        mPos.y  = ci::Rand::randInt( ci::app::getWindowHeight() * 0.4f );
    }
    
    ~BasicModule() {}
    
    void init()
    {
        registerParam( "radius", 5.0f, 0.0f, 60.0f );
        
        registerParam( "pos_x", &mPos.x, 0, ci::app::getWindowWidth() );
        registerParam( "pos_y", &mPos.y, 0, ci::app::getWindowHeight() );
        
        registerParam( "red", &mColor.r );
        registerParam( "green", &mColor.g );
        registerParam( "blue", &mColor.b );
    }
    
    // this gets called automatically by the timeline
    void update()
    {
//        ci::app::console() << "radius: " << getParamValue("radius") << std::endl; 
    }
    
    // this needs to be called explicitly
    void render()
    {
        ci::gl::color( mColor );
        ci::gl::drawSolidCircle( mPos, getParamValue("radius") );
        ci::gl::drawStringCentered( ci::toString( getParamValue("radius") ), mPos, ci::Color::white() );
    }
  
    void activeChanged(bool active)
    {
        printf("Module active: %d\n", active ? 1 : 0);
    }
    
private:
    
    ci::Color       mColor;
    ci::Vec2f       mPos;
};