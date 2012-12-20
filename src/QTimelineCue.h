/*
 *  QTimelineCue.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
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
    
    friend class QTimelineManager;
    
public:
    
    QTimelineCue( QTimeline *qTimeline, QTimelineCueManager *cueManager, std::string name = "untitled cue", double startTime = 0.0f, double duration = 1.0f );
    
    ~QTimelineCue() {}
    
    double getStartTime() { return mStartTime; }

    double getEndTime() { return mStartTime + mDuration; }
    
    std::string getName() { return mName; }
    
    void render();
    
    void dragHandle( float deltaT, float prevEndTime, float nextStartTime  );
    
    bool mouseMove( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    ci::XmlTree getXmlNode()
    {
        ci::XmlTree node( "cue", "" );
        node.setAttribute( "name",      mName );
        node.setAttribute( "startTime", mStartTime );
        node.setAttribute( "duration",  mDuration );
        return node;
    }
    
    void loadXmlNode( ci::XmlTree node ) {}
    
private:
    
    void findModuleBoundaries( float *prevEndTime, float *nextStartTime );
    
    void dragWidget( float deltaT, float prevEndTime, float nextStartTime  );

    void menuEventHandler( QTimelineMenuItem* item );
    
    void initMenu();
    
    void updateLabel();
    
protected:
    
    QTimeline               *mQTimeline;
    QTimelineCueManager     *mCueManager;
    
    std::string             mName;
    double                  mStartTime;
    double                  mDuration;
    
    bool                    mIsMouseOn;
};

#endif