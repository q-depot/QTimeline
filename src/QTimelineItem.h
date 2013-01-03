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
#include "QTimelineParam.h"

typedef std::shared_ptr<class QTimelineItem>      QTimelineItemRef;
typedef std::shared_ptr<class QTimelineTrack>     QTimelineTrackRef;


class QTimelineItem : public ci::TimelineItem
{
    
protected:
    
    QTimelineItem( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    {
        ci::app::console() << "QTimelineItem()" << std::endl;
        
        setAutoRemove(false);
        
        mParentTrack        = trackRef;
        mParent             = ciTimeline;
        
        setStartTime( startTime );
        setDuration( duration );
    }
    
public:
    
//    static QTimelineItemRef create( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
//    {
//          return QTimelineItemRef( new QTimelineItem( startTime, duration, trackRef, ciTimeline ) );
//    }
    
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





//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


/*
#include "cinder/TimelineItem.h"
#include "QTimelineWidgetWithHandles.h"
#include "QTimelineModuleItem.h"
#include "QTimelineParam.h"

class QTimelineTrack;
class QTimelineModule


typedef std::shared_ptr<class QTimelineModuleItem>      QTimelineModuleItemRef;
typedef std::shared_ptr<class QTimelineModule>          QTimelineModuleRef;
typedef std::shared_ptr<class QTimelineTrack>           QTimelineTrackRef;


class QTimelineModuleItem : public ci::TimelineItem, public QTimelineWidgetWithHandles
{
    
    friend class QTimeline;
    friend class QTimelineTrack;
    friend class QTimelineModule;
    friend class QTimelineParam;
    
public:
     
    
private:
    
    
private:
    
    QTimelineTrackRef               mParentTrack;
    
    QTimelineParamRef               mMouseOnParam;
    std::vector<QTimelineParamRef>  mParams;
    
    ci::Rectf                       mWidgetRect;    // the rect of the entire widget, size changes in accordance with the params rendered, when only the module is rendered this rect is equal to mRect
    
    QTimelineModuleRef              mTargetModuleRef;
    
    
private:
    // disallow
    //    QTimelineModuleItem(const QTimelineModuleItem&);
    //    QTimelineModuleItem& operator=(const QTimelineModuleItem&);
    
};


#endif
*/