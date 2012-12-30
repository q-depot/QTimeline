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
#include "QTimeline.h"

//class QTimelineTrack;
//class QTimelineModule;


#define TIMELINE_MODULE_HEIGHT          27      // the height in px


typedef std::shared_ptr<class QTimelineItem>        QTimelineItemRef;


class QTimelineItem : public ci::TimelineItem, public QTimelineWidgetWithHandles
{
    
    friend class QTimeline;
    friend class QTimelineTrack;
    friend class QTimelineModule;
    friend class QTimelineParam;
    
public:
    
    void init( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *timeline )
    {
        setAutoRemove(false);
        
        mParentTrack        = trackRef;
        mParent             = timeline;
        
        setStartTime( startTime );
        setDuration( duration );
        
        mBgColor            = QTimeline::mModulesBgCol;
        mBgOverColor        = QTimeline::mModulesBgOverCol;
        mTextColor          = QTimeline::mModulesTextCol;
        mHandleColor        = QTimeline::mModulesHandleCol;
        mHandleOverColor    = QTimeline::mModulesHandleOverCol;
        
        // init rect width
        setRect( Rectf( mParentTrack->mQTimeline->getPosFromTime( getStartTime() ), 0,
                        mParentTrack->mQTimeline->getPosFromTime( getEndTime() ), 0 ) );
        
        updateLabel();
        
        initMenu();
    
    }
    
    virtual ~QTimelineItem() {}
    
    virtual void update( float relativeTime );
    
    virtual void clear();
    
    virtual void start( bool reverse ) {}
    
    virtual void complete( bool reverse ) {}
    
    virtual void reverse() {}
    
    virtual ci::TimelineItemRef clone() const
    {
        return ci::TimelineItemRef( new QTimelineItem(*this) );
    }
    
    // FIX THIS!
    virtual ci::TimelineItemRef cloneReverse() const
    {
        return ci::TimelineItemRef( new QTimelineItem(*this) );
        //        Timeline *result = new Timeline( *this );
        //        for( s_iter iter = result->mItems.begin(); iter != result->mItems.end(); ++iter ) {
        //            iter->second->reverse();
        //            iter->second->mStartTime = mDuration + ( mDuration - ( iter->second->mStartTime + iter->second->mDuration ) );
        //        }
        //        return TimelineItemRef( result );
    }
    
    virtual QTimelineItemRef	thisRef()
    {
        ci::TimelineItemRef thisTimelineItem    = TimelineItem::thisRef();
		QTimelineItemRef  result              = std::static_pointer_cast<QTimelineItem>( thisTimelineItem );
		return result;
	}
    
    virtual void render( bool mouseOver );
    
    virtual bool mouseMove( ci::app::MouseEvent event );
    
    virtual bool mouseDown( ci::app::MouseEvent event );
    
    virtual bool mouseUp( ci::app::MouseEvent event );
    
    virtual bool mouseDrag( ci::app::MouseEvent event );
    
    
    virtual bool isTrackOpen();
    
    virtual ci::XmlTree getXmlNode();
    
    virtual void loadXmlNode( ci::XmlTree node );
    
    bool isPlaying() { return hasStarted() && !isComplete(); }
    
    bool isInWindow( ci::Vec2f window )
    {
        if ( getEndTime() < window.x || getStartTime() > window.y )
            return false;
        
        return true;
    }
    
    void setWidgetRect( ci::Rectf rect ) { mWidgetRect = rect; }
    
    ci::Rectf getWidgetRect() { return mWidgetRect; }
    
    
private:
    
    virtual void menuEventHandler( QTimelineMenuItemRef item );
    
    virtual bool dragHandles( ci::app::MouseEvent event );
    
    virtual void dragWidget( ci::app::MouseEvent event );
    
    virtual void initMenu();
    
    void findModuleBoundaries( float *prevEndTime, float *nextStartTime );
    
    QTimelineItemRef getModuleRef();
    
    
private:
    
    QTimelineTrackRef               mParentTrack;
    QTimelineParamRef               mMouseOnParam;
    
    std::vector<QTimelineParamRef>  mParams;
    
    ci::Rectf                       mWidgetRect;    // the rect of the entire widget, size changes in accordance with the params rendered, when only the module is rendered this rect is equal to mRect
    
    
private:
    // disallow
    QTimelineItem(const QTimelineItem&);
    QTimelineItem& operator=(const QTimelineItem&);
    
};


#endif