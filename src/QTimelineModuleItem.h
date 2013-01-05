/*
 *  QTimelineModuleItem.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_MODULE_ITEM
#define QTIMELINE_MODULE_ITEM

#pragma once

#include "QTimelineItem.h"

#define TIMELINE_MODULE_HEIGHT          20      // the height in px

typedef std::shared_ptr<class QTimelineModuleItem>      QTimelineModuleItemRef;
typedef std::shared_ptr<class QTimelineModule>          QTimelineModuleRef;

//class QTimelineModule;

class QTimelineModuleItem : public QTimelineItem
{
    friend class QTimelineModule;
    
public:
    
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
    
    ci::XmlTree getXmlNode();
    
//    void loadXmlNode( ci::XmlTree node );
    
public: // custom methods
    
    void resetTarget() { mTargetModuleRef.reset(); }
    
    std::string getTargetType();
    
private:
    
    QTimelineModuleItem( float startTime, float duration, QTimelineModuleRef targetRef, QTimelineTrackRef trackRef, ci::Timeline *ciTimeline );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
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