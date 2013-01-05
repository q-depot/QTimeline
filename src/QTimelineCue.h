/*
 *  QTimelineCue.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_CUE
#define QTIMELINE_CUE

#pragma once

#include "QTimelineWidgetWithHandles.h"

class QTimeline;
class QTimelineCueManager;

typedef std::shared_ptr<class QTimelineCue>		QTimelineCueRef;

class QTimelineCue : public QTimelineWidgetWithHandles
{
    
    friend class QTimelineCueManager;
    
public:
    
    QTimelineCue( QTimeline *qTimeline, QTimelineCueManager *cueManager, std::string name = "untitled cue", double startTime = 0.0f, double duration = 1.0f );
    
    ~QTimelineCue() {}
    
    double getStartTime() { return mStartTime; }

    double getEndTime() { return mStartTime + mDuration; }
    
    double getDuration() { return mDuration; }
    
    void setStartTime( double time ) { mStartTime = time; }
    
    void setEndTime( double time ) { mDuration = time - mStartTime; }
    
    void setDuration( double time ) { mDuration = time; }
    
    std::string getName() { return mName; }
    
    void render();
    
    ci::XmlTree getXmlNode();
    
    float getTimeNorm();
    
private:
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );

    bool dragHandles( ci::app::MouseEvent event );
    
    void dragWidget( ci::app::MouseEvent event );
    
    void findModuleBoundaries( float *prevEndTime, float *nextStartTime );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    void initMenu();
    
    void updateLabel();
    
protected:
    
    QTimeline               *mQTimeline;
    QTimelineCueManager     *mCueManager;
    
//    std::string             mName;
    double                  mStartTime;
    double                  mDuration;
    
    bool                    mIsMouseOn;
};

#endif