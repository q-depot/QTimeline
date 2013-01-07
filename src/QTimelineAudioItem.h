/*
 *  QTimelineAudioItem.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_AUDIO_ITEM
#define QTIMELINE_AUDIO_ITEM

#pragma once

#include "QTimelineItem.h"
#include "cinder/audio/Output.h"
//#include "FMOD.hpp"

typedef std::shared_ptr<class QTimelineAudioItem>      QTimelineAudioItemRef;


class QTimelineAudioItem : public QTimelineItem
{
    
public:
    
    static QTimelineAudioItemRef create( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    {
        return QTimelineAudioItemRef( new QTimelineAudioItem( startTime, duration, trackRef, ciTimeline ) );
    }
    
    ~QTimelineAudioItem() {}
    
    void update( float relativeTime );
    
    void render( bool mouseOver );
    
    void clear();
    
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
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
private:
    
    QTimelineAudioItem ( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    void initMenu();
    
    void loadAudioTrack();
    
private:
        
    ci::audio::TrackRef     	mTrack;
    std::string                 mTrackFilename;
    
    ci::audio::PcmBuffer32fRef  mPcmBuffer;
    
//	FMOD::System	*mSystem;
//    FMOD::Sound    	*mSound;
//	FMOD::Channel	*mChannel;
    
    ci::PolyLine<ci::Vec2f>     mLeftBufferLine;
    ci::PolyLine<ci::Vec2f>     mRightBufferLine;
    
};


#endif

