/*
 *  QTimelineAudioItem.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_AUDIO_ITEM
#define QTIMELINE_AUDIO_ITEM

#pragma once

//#include "cinder/TimelineItem.h"
#include "QTimelineItem.h"
//#include "QTimelineWidgetWithHandles.h"


typedef std::shared_ptr<class QTimelineAudioItem>      QTimelineAudioItemRef;


class QTimelineAudioItem : public QTimelineItem//, public QTimelineWidgetWithHandles
{
    
public:
    
    static QTimelineAudioItemRef create( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    {
        return QTimelineAudioItemRef( new QTimelineAudioItem( startTime, duration, trackRef, ciTimeline ) );
    }
    
    ~QTimelineAudioItem() {}
    
    void update( float relativeTime )
    {
        ci::app::console() << "QTimelineAudioItem::update() " << relativeTime << std::endl;
    }
    
    void clear() {}
    
    ci::TimelineItemRef clone() const
    {
        return ci::TimelineItemRef( new QTimelineAudioItem(*this) );
    }
    
    ci::TimelineItemRef cloneReverse() const
    {
        return ci::TimelineItemRef( new QTimelineAudioItem(*this) );
    }
    
    QTimelineItemRef	thisRef()
    {
        ci::TimelineItemRef thisTimelineItem    = TimelineItem::thisRef();
		QTimelineItemRef  result              = std::static_pointer_cast<QTimelineItem>( thisTimelineItem );
		return result;
	}
    
    bool mouseMove( ci::app::MouseEvent event ) { return false; }
    
    bool mouseDown( ci::app::MouseEvent event ) { return false; }
    
    bool mouseUp( ci::app::MouseEvent event ) { return false; }
    
    bool mouseDrag( ci::app::MouseEvent event ) { return false; }

private:
    
    QTimelineAudioItem ( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    : QTimelineItem( startTime, duration, "QTimelineAudioItem", "track name!", trackRef, ciTimeline ) {}
    
    void menuEventHandler( QTimelineMenuItemRef item ) {}
    
    void findModuleBoundaries( float *prevEndTime, float *nextStartTime ) {}
    
    bool dragHandles( ci::app::MouseEvent event ) { return false; }
    
    void dragWidget( ci::app::MouseEvent event ) {}
    
    void initMenu() {}
};


#endif

