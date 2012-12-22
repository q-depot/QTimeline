/*
 *  QTimelineModuleItem.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_MODULE_ITEM
#define QTIMELINE_MODULE_ITEM

#pragma once

#include "cinder/TimelineItem.h"
#include "QTimelineWidgetWithHandles.h"
#include "QTimelineModuleItem.h"
#include "QTimelineParam.h"

class QTimelineTrack;
class QTimelineModule;


#define TIMELINE_MODULE_HEIGHT          27      // the height in px


typedef std::shared_ptr<class QTimelineModuleItem>      QTimelineModuleItemRef;
typedef std::shared_ptr<class QTimelineModule>          QTimelineModuleRef;


class QTimelineModuleItem : public ci::TimelineItem, public QTimelineWidgetWithHandles
{
    
    friend class QTimeline;
    friend class QTimelineTrack;
    friend class QTimelineModule;
    friend class QTimelineParam;
    
public:
    
    static QTimelineModuleItemRef create( QTimelineModule *targetModule, float startAt, float duration, QTimelineTrackRef trackRef, ci::Timeline *timeline )
    {
        return QTimelineModuleItemRef( new QTimelineModuleItem( targetModule, startAt, duration, trackRef, timeline ) );
    }
  
    ~QTimelineModuleItem();
    
    virtual void update( float relativeTime );
    
    void start( bool reverse ) {}
    
    void complete( bool reverse ) {}
    
    void reverse() {}
    
    ci::TimelineItemRef clone() const
    {
        return ci::TimelineItemRef( new QTimelineModuleItem(*this) );
    }
    
    // FIX THIS!
    ci::TimelineItemRef cloneReverse() const
    {
        return ci::TimelineItemRef( new QTimelineModuleItem(*this) );
        //        Timeline *result = new Timeline( *this );
        //        for( s_iter iter = result->mItems.begin(); iter != result->mItems.end(); ++iter ) {
        //            iter->second->reverse();
        //            iter->second->mStartTime = mDuration + ( mDuration - ( iter->second->mStartTime + iter->second->mDuration ) );
        //        }
        //        return TimelineItemRef( result );
    }
    
    QTimelineModuleItemRef	thisRef()
    {
        ci::TimelineItemRef thisTimelineItem    = TimelineItem::thisRef();
		QTimelineModuleItemRef  result              = std::static_pointer_cast<QTimelineModuleItem>( thisTimelineItem );
		return result;
	}
    
    bool isPlaying() { return hasStarted() && !isComplete(); }
    
    bool isInWindow( ci::Vec2f window )
    {
        if ( getEndTime() < window.x || getStartTime() > window.y )
            return false;
        
        return true;
    }
    
    void registerParam( const std::string name, float initVal = 0.0f, float minVal = 0.0f, float maxVal = 1.0f );

    void registerParam( const std::string name, float *var, float minVal = 0.0f, float maxVal = 1.0f );
    
    float getParamValue( const std::string &name );
    
    void setWidgetRect( ci::Rectf rect ) { mWidgetRect = rect; }
    
    ci::Rectf getWidgetRect() { return mWidgetRect; }
    
    void render( bool mouseOver );    
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    bool isMouseOnParam( QTimelineParamRef ref );
    
    bool isMouseOnKeyframe( QTimelineParamRef paramRef, QTimelineKeyframeRef keyframeRef );
    
    bool isTrackOpen();
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
    std::string getType();
    
    
private:
    
    QTimelineModuleItem( QTimelineModule *targetModule, float startAt, float duration, QTimelineTrackRef trackRef, ci::Timeline *parent );
    
    void menuEventHandler( QTimelineMenuItem* item );
    
    bool updateAtLoopStart() { return false; }
    
    void findModuleBoundaries( float *prevEndTime, float *nextStartTime );
    
    bool dragHandles( ci::app::MouseEvent event );
    
    void dragWidget( ci::app::MouseEvent event );
    
    QTimelineModuleItemRef getModuleRef();
    
    void initMenu();
    
    QTimelineParamRef findParamByName( std::string name );
    
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