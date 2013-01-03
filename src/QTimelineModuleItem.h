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

//#include "QTimelineWidgetWithHandles.h"
#include "QTimelineItem.h"

class QTimelineModule;

#define TIMELINE_MODULE_HEIGHT          27      // the height in px


typedef std::shared_ptr<class QTimelineModuleItem>      QTimelineModuleItemRef;
typedef std::shared_ptr<class QTimelineModule>          QTimelineModuleRef;


class QTimelineModuleItem : public QTimelineItem//, public QTimelineWidgetWithHandles
{
    friend class QTimelineModule;
    
public:     // QTimelineItem
    
    static QTimelineModuleItemRef create( float startTime, float duration, QTimelineModuleRef targetRef, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    {
        return QTimelineModuleItemRef( new QTimelineModuleItem( startTime, duration, targetRef, trackRef, ciTimeline ) );
    }
    
    ~QTimelineModuleItem();
    
    void update( float relativeTime );
    
    void render( bool mouseOver );
    
    void clear();
    
    ci::TimelineItemRef clone() const
    {
        return ci::TimelineItemRef( new QTimelineModuleItem(*this) );
    }
    
    ci::TimelineItemRef cloneReverse() const
    {
        return ci::TimelineItemRef( new QTimelineModuleItem(*this) );
    }
    
    QTimelineItemRef	thisRef()
    {
        ci::TimelineItemRef thisTimelineItem    = TimelineItem::thisRef();
		QTimelineItemRef  result                = std::static_pointer_cast<QTimelineItem>( thisTimelineItem );
		return result;
	}
    
public:     // Widget
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
    
public:     // custom functions
    
    void resetTarget() { mTargetModuleRef.reset(); }
    
private:
    
    QTimelineModuleItem( float startTime, float duration, QTimelineModuleRef targetRef, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    void findModuleBoundaries( float *prevEndTime, float *nextStartTime );
    
    bool dragHandles( ci::app::MouseEvent event );
    
    void dragWidget( ci::app::MouseEvent event );
    
    QTimelineModuleItemRef getModuleRef();
    
    void initMenu();
    
private:
    
    QTimelineModuleRef              mTargetModuleRef;
    
    
private:
    // disallow
//    QTimelineModuleItem(const QTimelineModuleItem&);
//    QTimelineModuleItem& operator=(const QTimelineModuleItem&);
    
};


#endif