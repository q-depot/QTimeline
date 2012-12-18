/*
 *  QTimelineKeyframe.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_KEYFRAME
#define QTIMELINE_KEYFRAME

#pragma once

#include "cinder/Tween.h"
#include "cinder/Easing.h"

#define TIMELINE_KEYFRAME_SIZE  4


typedef std::shared_ptr<class QTimelineKeyframe>    QTimelineKeyframeRef;

class QTimelineModuleItem;


class QTimelineKeyframe {

    friend class QTimelineTrack;
    friend class QTimelineParam;
    
public:
    
    QTimelineKeyframe(double time, float value, std::function<float (float)> fn ) : mTime(time), mValue(value), mFn(fn) {}

    ~QTimelineKeyframe() {}
    
    double getTime() { return mTime; }
    
    float getValue() { return mValue; }
    
    void set( double time, float value )
    {
        mTime   = time;
        mValue  = value;
    }
    
    void setEasing( std::function<float (float)> fn ) { mFn = fn; }
    
    ci::Vec2f getAbsolutePosition( ci::Rectf paramRect, float max, float min, double startTime, double endTime )
    {
        ci::Vec2f pos;
        pos.x   =   ( mTime - startTime ) / ( endTime - startTime );
        pos.y   =   ( mValue - min ) / ( max - min );
        pos     *=  paramRect.getSize();
        pos.x   +=  paramRect.getX1();
        pos.y   =   paramRect.getY2() - pos.y;
        
        return pos;
    }
    
    std::string getFormattedStr()
    {
        char buff[100];
        sprintf( buff, "%.3f", mValue );
        return buff;
    }
    
    ci::XmlTree getXmlNode()
    {
        ci::XmlTree node( "kf", "" );
        node.setAttribute( "value", mTime );
        node.setAttribute( "time", mValue );
        node.setAttribute( "fn", "TODO!" );
        
        return node;
    }
    
    void loadXmlNode( ci::XmlTree node ) {}
    
    
private:

    double  mTime;
    float   mValue;
    
	std::function<float (float)>	mFn;

};

#endif