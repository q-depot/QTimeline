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


class QTimelineModuleItem : public QTimelineItem
{
    friend class QTimelineModule;
    
public:

    static QTimelineModuleItemRef create( std::string name, float startTime, float duration, QTimelineTrackRef trackRef )
    {
        return QTimelineModuleItemRef( new QTimelineModuleItem( name, startTime, duration, trackRef ) );
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

    
private:
    
    QTimelineModuleItem( std::string name, float startTime, float duration, QTimelineTrackRef trackRef );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    QTimelineModuleItemRef getModuleRef();
    
    void initMenu();
    
    
    
private:
    // disallow
//    QTimelineModuleItem(const QTimelineModuleItem&);
//    QTimelineModuleItem& operator=(const QTimelineModuleItem&);
    
};


#endif