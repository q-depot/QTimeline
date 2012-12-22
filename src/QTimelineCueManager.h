/*
 *  QTimelineCueManager.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_CUE_MANAGER
#define QTIMELINE_CUE_MANAGER

#pragma once

#include "QTimelineWidget.h"
#include "QTimelineCue.h"

#define TIMELINE_CUELIST_HEIGHT     30


class QTimeline;

class QTimelineCueManager : public QTimelineWidget
{
    
    friend class QTimelineCue;
    
public:
    
    QTimelineCueManager( QTimeline *qTimeline );
    
    ~QTimelineCueManager()
    {
        clear();
    }
    
    void clear()
    {
        mCueList.clear();
    }
    
    void render();
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    void addCue( std::string name, double startTime, double duration );
    
    void deleteCue( QTimelineCue *cue );
    
    std::vector<QTimelineCueRef> getCueList() { return mCueList; }
    
    QTimelineCueRef getCurrentCue() { return mCurrentCue; }
        
    void playCue();
    
    bool isTimeOnCue();

    double getCueEndTime() { return mCurrentCue->getEndTime(); }
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
private:
    
    void menuEventHandler( QTimelineMenuItem* item );
    
    void initMenu();
    
private:
    
    QTimeline                       *mQTimeline;
    std::vector<QTimelineCueRef>    mCueList;
    QTimelineCueRef                 mCurrentCue;
    
};

#endif


