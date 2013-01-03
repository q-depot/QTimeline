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
    
    void render( bool mouseOver )
    {
        // render bg rect
        glBegin( GL_QUADS );
                ci::gl::color( ci::Color::white() );
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        ci::gl::vertex( mRect.getLowerLeft() );
        glEnd();
        
        // render handles
        if ( mouseOver )
            renderHandles();
        
        ci::gl::color( ci::ColorA( 1.0f, 1.0f, 1.0f, 0.08f ) );
        glBegin( GL_LINE_STRIP );
        ci::gl::vertex( mRect.getLowerLeft() );
        ci::gl::vertex( mRect.getUpperLeft() );
        ci::gl::vertex( mRect.getUpperRight() );
        ci::gl::vertex( mRect.getLowerRight() );
        glEnd();
        
        // render name
        ci::gl::color( ci::Color( 1.0f, 1.0f, 0.0f ) );
        mFont->drawString( getLabel(), mRect.getCenter() + mLabelStrSize * ci::Vec2f( -0.5f, 0.3f ) );
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

