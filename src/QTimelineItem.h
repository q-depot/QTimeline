/*
 *  QTimelineItem.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_ITEM
#define QTIMELINE_ITEM

#pragma once

#include "cinder/TimelineItem.h"
#include "QTimelineWidgetWithHandles.h"
#include "QTimelineParam.h"

typedef std::shared_ptr<class QTimelineItem>      QTimelineItemRef;
typedef std::shared_ptr<class QTimelineTrack>     QTimelineTrackRef;


class QTimelineItem : public ci::TimelineItem, public QTimelineWidgetWithHandles
{
    
protected:
    
    QTimelineItem( float startTime, float duration, std::string type, std::string name, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline ) : QTimelineWidgetWithHandles()
    {
        setAutoRemove(false);
        
        mParentTrack        = trackRef;
        mParent             = ciTimeline;
        mType               = type;
        mName               = name;
        
        setStartTime( startTime );
        setDuration( duration );
    }
    
public:
    
    virtual ~QTimelineItem() {}
    
    virtual void update( float relativeTime )
    {
        ci::app::console() << "QTimelineItem::update() " << relativeTime << std::endl;
    }
    
    virtual void render( bool mouseOver ) {}
    
    virtual void clear() {}
    
    virtual void start( bool reverse ) {}
    
    virtual void complete( bool reverse ) {}
    
    virtual void reverse() {}
    
    virtual ci::TimelineItemRef clone() const
    {
        return ci::TimelineItemRef( new QTimelineItem(*this) );
    }
    
    virtual ci::TimelineItemRef cloneReverse() const
    {
        return ci::TimelineItemRef( new QTimelineItem(*this) );
    }
    
    virtual QTimelineItemRef	thisRef()
    {
        ci::TimelineItemRef thisTimelineItem    = TimelineItem::thisRef();
		QTimelineItemRef  result                = std::static_pointer_cast<QTimelineItem>( thisTimelineItem );
		return result;
	}
    
protected:
    
    virtual void findModuleBoundaries( float *prevEndTime, float *nextStartTime ) {}
    
    
public:
    
    bool isPlaying() { return hasStarted() && !isComplete(); }
    
    bool isInWindow( ci::Vec2f window )
    {
        if ( getEndTime() < window.x || getStartTime() > window.y )
            return false;
        
        return true;
    }
    
    void registerParam( const std::string name, float initVal, float minVal, float maxVal )
    {
        registerParam( name, new float(initVal), minVal, maxVal );
    }
    
    void registerParam( const std::string name, float *var, float minVal, float maxVal )
    {
        for( size_t k=0; k < mParams.size(); k++ )
            if ( mParams[k]->getName() == name )
                mParams[k]->swapPointer( var );

        mParams.push_back( QTimelineParamRef( new QTimelineParam( thisRef(), name, var, minVal, maxVal ) ) );
    }
    
    float getParamValue( const std::string &name )
    {
        for( size_t k=0; k < mParams.size(); k++ )
            if ( mParams[k]->getName() == name )
                return mParams[k]->getValue();
        
        return 0.0f;
    }
    
    bool isMouseOnParam()
    {
        return mMouseOnParam ? true : false;
    }
    
    bool isMouseOnParam( QTimelineParamRef ref )
    {
        return ref == mMouseOnParam;
    }
    
    bool isMouseOnKeyframe( QTimelineParamRef paramRef, QTimelineKeyframeRef keyframeRef )
    {
        return paramRef == mMouseOnParam && paramRef->isMouseOnKeyframe(keyframeRef);
    }
    
    std::string getType() { return mType; } 
    
    size_t getNumParams() { return mParams.size(); }
    
    std::vector<QTimelineParamRef> getParams() { return mParams; }
    
    void setParentTrack( QTimelineTrackRef ref ) { mParentTrack.swap( ref ); }
    
    QTimelineTrackRef getParentTrack() { return mParentTrack; }
    
    
protected:
    
    QTimelineParamRef findParamByName( std::string name )
    {
        for( size_t k=0; k < mParams.size(); k++ )
            if ( mParams[k]->getName() == name )
                return mParams[k];
        
        QTimelineParamRef nullPtr;
        return nullPtr;
    }
    
protected:
    
    std::string                     mType;
    QTimelineTrackRef               mParentTrack;
    
    QTimelineParamRef               mMouseOnParam;
    std::vector<QTimelineParamRef>  mParams;
};


#endif

