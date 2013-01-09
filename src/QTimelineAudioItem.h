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
#include "bass.h"

typedef std::shared_ptr<class QTimelineAudioItem>      QTimelineAudioItemRef;

#define AUDIO_WAVEFORM_PRECISION    0.050f    // plot the waveform every N seconds

class QTimelineAudioItem : public QTimelineItem
{
    
public:
    
    static QTimelineAudioItemRef create( float startTime, float duration, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    {
        return QTimelineAudioItemRef( new QTimelineAudioItem( startTime, duration, trackRef, ciTimeline ) );
    }
    
    ~QTimelineAudioItem()
    {
        clear();
    }
    
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
    
    void cacheWaveForm();
    
    void onTimeChange();
    
private:
    
    std::string                 mTrackFilename;
    double                      mTrackDuration;
    HSTREAM                     mAudioHandle;
    std::vector<float>          mWaveFormLeft;
    std::vector<float>          mWaveFormRight;
    
};


#endif
