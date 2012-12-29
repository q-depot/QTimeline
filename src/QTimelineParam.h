/*
 *  QTimelineParam.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2012 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_PARAM
#define QTIMELINE_PARAM

#pragma once

//#include "cinder/Tween.h"
#include "cinder/Easing.h"
#include "QTimelineKeyframe.h"

#define TIMELINE_PARAM_HEIGHT   45

typedef std::shared_ptr<class QTimelineParam>       QTimelineParamRef;

class QTimelineModuleItem;

class QTimelineParam : public QTimelineWidget
{
    
    friend class QTimeline;
    friend class QTimelineTrack;
    friend class QTimelineModuleItem;
    
public:
    
    QTimelineParam( QTimelineModuleItemRef itemRef, const std::string &name, float *var, float minVal, float maxVal, double startTime );
    
    ~QTimelineParam();
    
    ci::Rectf render( ci::Rectf prevElementRect );
    
    void update( double relativeTime );
    
    void renderKeyframes();
    
    float getValue()    { return *mVar; }
    float *getRef()     { return mVar; }
    
    void setValue( float val ) { *mVar = ci::math<float>::clamp( val, mMin, mMax ); }
    
    float getMin()    { return mMin; }
    float getMax()    { return mMax; }
    
    void addKeyframe( double time, float value, std::function<float (float)> fn, std::string fnStr );
    
    void addKeyframe( double time, float value );
    
    void removeKeyframe( QTimelineKeyframeRef ref );
    
    std::vector<QTimelineKeyframeRef> getKeyframes() { return mKeyframes; }
    
    size_t                          getKeyframesN() { return mKeyframes.size(); }
    
    QTimelineKeyframeRef            getKeyframeRef( size_t n ) { return mKeyframes[n]; }
    
    bool mouseMove( ci::app::MouseEvent event );

    bool mouseUp( ci::app::MouseEvent event );
    
    bool mouseDown( ci::app::MouseEvent event );
    
    bool mouseDrag( ci::app::MouseEvent event );
    
    bool isMouseOnKeyframe( QTimelineKeyframeRef ref ) { return mMouseOnKeyframe == ref; };
    
    void updateKeyframesPos( float deltaT );
    
    ci::XmlTree getXmlNode();
    
    void loadXmlNode( ci::XmlTree node );
    
    void menuEventHandler( QTimelineMenuItemRef item );
    
    
private:
    
    ci::Vec2f   getRelPosNorm( ci::Vec2f pos );
    
    float getPosValue( float yPos );
    
    void findKeyframesInSelection();
    
    ci::Vec2f getKeyframePos( QTimelineKeyframeRef keyframeRef );
    
    void initMenu();
    
    
protected:
    
    float       *mVar;
    float       mMax;
    float       mMin;
    
    QTimelineKeyframeRef                mMouseOnKeyframe;
    std::vector<QTimelineKeyframeRef>   mKeyframes;
    
    QTimelineModuleItemRef              mParentModule;
    
    ci::Vec2f                           mMousePos;
    bool                                mIsOnSelection;
    std::vector<QTimelineKeyframeRef>   mKeyframesSelection;
    
    ci::ColorA                          mKeyframesBgCol;
    ci::ColorA                          mKeyframesBgOverCol;
    ci::ColorA                          mKeyframesBgSelectedCol;
    ci::ColorA                          mKeyframesGraphCol;
    
    std::function<float (float)>        mDefaultEasing;
    std::string                         mDefaultEasingStr;
};

#endif