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
#include <boost/algorithm/string.hpp>

typedef std::shared_ptr<class QTimelineAudioItem>      QTimelineAudioItemRef;

#define AUDIO_WAVEFORM_PRECISION    0.050f    // plot the waveform every N seconds


class SupportedFormat
{
    
public:
    
    SupportedFormat( std::string formats )
    {
        std::vector<std::string> strs;
        boost::split( strs, formats, boost::is_any_of(" ") );
        
        for( size_t k=0; k < strs.size(); k++ )
            mFormats.push_back( strs[k] );
    }
    
    bool isSupported( std::string format )
    {
        for( size_t k=0; k < mFormats.size(); k++ )
            if ( mFormats[k] == format )
                return true;
        
        return false;
    }
    
private:
    
    std::vector<std::string>    mFormats;
    
};


class QTimelineAudioItem : public QTimelineItem
{
    
public:
    
    static QTimelineAudioItemRef create( float startTime, float duration, std::string filename, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline )
    {
        return QTimelineAudioItemRef( new QTimelineAudioItem( startTime, duration, filename, trackRef, ciTimeline ) );
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
    
    float getHeight()
    {
        if ( mParentTrack->isOpen() )
            return  TIMELINE_ITEM_HEIGHT + TIMELINE_WIDGET_PADDING +
                    ( 1 + mParams.size() ) * ( TIMELINE_PARAM_HEIGHT + TIMELINE_WIDGET_PADDING ); // waveform + params
        else
            return TIMELINE_ITEM_HEIGHT + TIMELINE_WIDGET_PADDING;
    }
    
    
private:
    
    QTimelineAudioItem ( float startTime, float duration, std::string filename, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    void initMenu();
    
    void loadAudioTrack( std::string filename );
    
    void cacheWaveForm();
    
    void onTimeChange();
    
    void renderWaveForm( ci::Rectf rect );
    
    
public:
    
    static SupportedFormat supportedFormats;
    
    
private:
    
    ci::fs::path                mFilePath;
    double                      mTrackDuration;
    HSTREAM                     mAudioHandle;
    std::vector<float>          mWaveFormLeft;
    std::vector<float>          mWaveFormRight;
    
};


#endif
