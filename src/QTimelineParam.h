/*
 *  QTimelineParam.h
 *
 *  Created by Andrea Cuius
 *  Nocte Studio Ltd. Copyright 2013 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QTIMELINE_PARAM
#define QTIMELINE_PARAM

#pragma once

#include "cinder/Easing.h"
#include "QTimelineKeyframe.h"

#define TIMELINE_PARAM_HEIGHT   45

typedef std::shared_ptr<class QTimelineParam>       QTimelineParamRef;
typedef std::shared_ptr<class QTimelineItem>        QTimelineItemRef;


class QTimelineParam : public QTimelineWidget
{
    
    friend class QTimeline;
    friend class QTimelineTrack;
    
public:
  
    QTimelineParam( QTimelineItemRef itemRef, const std::string &name, float *var, float minVal, float maxVal );
    
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
    
    // at the moment this function might leak,
    // if the param has been created pssing the pointer to the variable, we can't call delete on the pointer.
    // on the other hand if the variable has been created by the params(ie when the param is register with a name and without a variable),
    // we should call the delete on the pointer, otherwise it leaks.
    // TODO: mVar should be a shared_ptr
    void swapPointer( float *var ) { mVar = var; }
    
    
    void        setKeyframesBgCol( ci::ColorA col ) { mKeyframesBgCol = col; }
    ci::ColorA  getKeyframesBgCol() { return mKeyframesBgCol; }
    
    void        setKeyframesBgOverCol( ci::ColorA col ) { mKeyframesBgOverCol = col; }
    ci::ColorA  getKeyframesBgOverCol() { return mKeyframesBgOverCol; }
    
    void        setKeyframesBgSelectedCol( ci::ColorA col ) { mKeyframesBgSelectedCol = col; }
    ci::ColorA  getKeyframesBgSelectedCol() { return mKeyframesBgSelectedCol; }
    
    void        setKeyframesGraphCol( ci::ColorA col ) { mKeyframesGraphCol = col; }
    ci::ColorA  getKeyframesGraphCol() { return mKeyframesGraphCol; }
    
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
    
    QTimelineItemRef                    mParentModule;
    
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