// rdmarkerview.cpp
//
// Widget for displaying/editing cut markers
//
//   (C) Copyright 2021 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QPen>
#include <QScrollBar>

#include "rdescape_string.h"
#include "rdmarkerview.h"

#define LEFT_MARGIN 20

RDMarkerHandle::RDMarkerHandle(RDMarkerHandle::PointerRole role,
			       PointerType type,void *mkrview,
			       QGraphicsItem *parent)
  : QGraphicsPolygonItem(parent)
{
  setFlags(QGraphicsItem::ItemIsMovable);
  d_marker_view=mkrview;
  d_x_diff=0;

  switch(type) {
  case RDMarkerHandle::Start:
    d_triangle << QPointF(0.0,0.0) << QPointF(-16,8) << QPointF(-16,-8);
    d_big_triangle << QPointF(0.0,0.0) << QPointF(-32,16) << QPointF(-32,-16);
    break;

  case RDMarkerHandle::End:
    d_triangle << QPointF(0.0,0.0) << QPointF(16,8) << QPointF(16,-8);
    d_big_triangle << QPointF(0.0,0.0) << QPointF(32,16) << QPointF(32,-16);
    break;
  }
  setPolygon(d_triangle);
  setPen(QPen(RDMarkerHandle::pointerRoleColor(role)));
  setBrush(RDMarkerHandle::pointerRoleColor(role));
  d_name=RDMarkerHandle::pointerRoleTypeText(role);
  d_role=role;
  setToolTip(RDMarkerHandle::pointerRoleTypeText(role));
}


QString RDMarkerHandle::name() const
{
  return d_name;
}


RDMarkerHandle::PointerRole RDMarkerHandle::role() const
{
  return d_role;
}


void RDMarkerHandle::setMinimum(int pos,int ptr)
{
  d_minimum_pos=pos;
  d_minimum_ptr=ptr;
}


void RDMarkerHandle::setMaximum(int pos,int ptr)
{
  d_maximum_pos=pos;
  d_maximum_ptr=ptr;
}


bool RDMarkerHandle::isSelected() const
{
  return d_is_selected;
}


void RDMarkerHandle::setSelected(bool state)
{
  if(d_is_selected!=state) {
    if(state) {
      setPolygon(d_big_triangle);
    }
    else {
      setPolygon(d_triangle);
    }
    d_is_selected=state;
  }
}


void RDMarkerHandle::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
  RDMarkerView *view=static_cast<RDMarkerView *>(d_marker_view);

  if(e->button()==Qt::LeftButton) {
    e->accept();
    d_x_diff=pos().x()-e->scenePos().x();
    if(d_peers.size()==0) {
      QGraphicsScene *s=scene();
      QList<QGraphicsItem *> items=s->items();
      for(int i=0;i<items.size();i++) {
	if(items.at(i)->toolTip()==toolTip()) {
	  d_peers.push_back(items.at(i));
	}
      }
    }
    view->setSelectedMarker(d_role);
  }

  if(e->button()==Qt::RightButton) {
    e->ignore();
    view->processRightClick(d_role,e->screenPos());
  }
}


void RDMarkerHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
  RDMarkerView *view=static_cast<RDMarkerView *>(d_marker_view);

  //
  // Check Limit Stops
  //
  int limit_ptr=-1;
  int x=e->scenePos().x()+d_x_diff-LEFT_MARGIN;

  if((d_minimum_pos>=0)&&(x<d_minimum_pos)) {
    d_x_diff=d_minimum_pos-e->scenePos().x()+LEFT_MARGIN;
    x=d_minimum_pos;
    limit_ptr=d_minimum_ptr;
  }
  else {
    if((d_maximum_pos>=0)&&(x>d_maximum_pos)) {
      d_x_diff=d_maximum_pos-e->scenePos().x()+LEFT_MARGIN;
      x=d_maximum_pos;
      limit_ptr=d_maximum_ptr;
    }
  }

  //
  // Update Marker Graphic
  //
  for(int i=0;i<d_peers.size();i++) {
    QGraphicsItem *peer=d_peers.at(i);
    peer->setPos(e->scenePos().x()+d_x_diff,peer->pos().y());
  }

  if(limit_ptr<0) {  // Generate new pointer value
    int64_t pframes=(int64_t)(1152*x*view->shrinkFactor());
    view->updatePosition(d_role,(int)((int64_t)1000*pframes/
				      (int64_t)view->sampleRate()));
  }
  else {  // We're against a limit stop, so use the pointer value of the stop
    view->updatePosition(d_role,limit_ptr);
  }
}


void RDMarkerHandle::wheelEvent(QGraphicsSceneWheelEvent *e)
{
  /*
  if((e->buttons()&Qt::MiddleButton)!=0) {
    printf("wheelEvent(%d)\n",e->delta());
  }
  */
}


void RDMarkerHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *e)
{
  //  RDMarkerView *view=static_cast<RDMarkerView *>(d_marker_view);
}


QString RDMarkerHandle::pointerRoleText(PointerRole role)
{
  QString ret=QObject::tr("Unknown");

  switch(role) {
  case RDMarkerHandle::CutStart:
  case RDMarkerHandle::CutEnd:
    ret=QObject::tr("Length");
    break;

  case RDMarkerHandle::TalkStart:
  case RDMarkerHandle::TalkEnd:
    ret=QObject::tr("Talk");
    break;

  case RDMarkerHandle::SegueStart:
  case RDMarkerHandle::SegueEnd:
    ret=QObject::tr("Segue");
    break;

  case RDMarkerHandle::HookStart:
  case RDMarkerHandle::HookEnd:
    ret=QObject::tr("Hook");
    break;

  case RDMarkerHandle::FadeUp:
    ret=QObject::tr("Fade Up");
    break;

  case RDMarkerHandle::FadeDown:
    ret=QObject::tr("Fade Down");
    break;

  case RDMarkerHandle::LastRole:
    break;
  }

  return ret;
}


QString RDMarkerHandle::pointerRoleTypeText(RDMarkerHandle::PointerRole role)
{
  QString ret=QObject::tr("Unknown");

  switch(role) {
  case RDMarkerHandle::CutStart:
    ret=QObject::tr("Cut Start");
    break;

  case RDMarkerHandle::CutEnd:
    ret=QObject::tr("Cut End");
    break;

  case RDMarkerHandle::TalkStart:
    ret=QObject::tr("Talk Start");
    break;

  case RDMarkerHandle::TalkEnd:
    ret=QObject::tr("Talk End");
    break;

  case RDMarkerHandle::SegueStart:
    ret=QObject::tr("Segue Start");
    break;

  case RDMarkerHandle::SegueEnd:
    ret=QObject::tr("Segue End");
    break;

  case RDMarkerHandle::HookStart:
    ret=QObject::tr("Hook Start");
    break;

  case RDMarkerHandle::HookEnd:
    ret=QObject::tr("Hook End");
    break;

  case RDMarkerHandle::FadeUp:
    ret=QObject::tr("Fade Up");
    break;

  case RDMarkerHandle::FadeDown:
    ret=QObject::tr("Fade Down");
    break;

  case RDMarkerHandle::LastRole:
    break;
  }

  return ret;
}


QColor RDMarkerHandle::pointerRoleColor(RDMarkerHandle::PointerRole role)
{
  QColor ret;

  switch(role) {
  case RDMarkerHandle::CutStart:
  case RDMarkerHandle::CutEnd:
    ret=Qt::red;
    break;

  case RDMarkerHandle::TalkStart:
  case RDMarkerHandle::TalkEnd:
    ret=Qt::blue;
    break;

  case RDMarkerHandle::SegueStart:
  case RDMarkerHandle::SegueEnd:
    ret=Qt::cyan;
    break;

  case RDMarkerHandle::HookStart:
  case RDMarkerHandle::HookEnd:
    ret=Qt::magenta;
    break;

  case RDMarkerHandle::FadeUp:
  case RDMarkerHandle::FadeDown:
    ret=Qt::darkYellow;
    break;

  case RDMarkerHandle::LastRole:
    break;
  }

  return ret;
}


RDMarkerHandle::PointerType RDMarkerHandle::pointerType(RDMarkerHandle::PointerRole role)
{
  RDMarkerHandle::PointerType ret=RDMarkerHandle::Start;

  switch(role) {
  case RDMarkerHandle::CutStart:
  case RDMarkerHandle::TalkStart:
  case RDMarkerHandle::SegueStart:
  case RDMarkerHandle::HookStart:
  case RDMarkerHandle::FadeDown:
    ret=RDMarkerHandle::Start;
    break;

  case RDMarkerHandle::CutEnd:
  case RDMarkerHandle::TalkEnd:
  case RDMarkerHandle::SegueEnd:
  case RDMarkerHandle::HookEnd:
  case RDMarkerHandle::FadeUp:
    ret=RDMarkerHandle::End;
    break;

  case RDMarkerHandle::LastRole:
    break;
  }

  return ret;
}


RDMarkerView::RDMarkerView(int width,int height,QWidget *parent)
  : QWidget(parent)
{
  d_width=width;
  d_height=height;
  d_scene=NULL;
  d_sample_rate=rda->system()->sampleRate();
  d_right_margin=0;
  clear();

  d_view=new QGraphicsView(this);

  d_wave_factory=new RDWaveFactory(RDWaveFactory::MultiTrack);

  d_pointer_fields.push_back("START_POINT");
  d_pointer_fields.push_back("END_POINT");
  d_pointer_fields.push_back("TALK_START_POINT");
  d_pointer_fields.push_back("TALK_END_POINT");
  d_pointer_fields.push_back("SEGUE_START_POINT");
  d_pointer_fields.push_back("SEGUE_END_POINT");
  d_pointer_fields.push_back("HOOK_START_POINT");
  d_pointer_fields.push_back("HOOK_END_POINT");
  d_pointer_fields.push_back("FADEUP_POINT");
  d_pointer_fields.push_back("FADEDOWN_POINT");

  //
  // The Main Mouse Menu
  //
  d_main_menu=new QMenu(this);
  connect(d_main_menu,SIGNAL(aboutToShow()),this,SLOT(updateMenuData()));
  d_delete_marker_action=d_main_menu->
    addAction(tr("Delete Marker"),this,SLOT(deleteMarkerData()));
  d_main_menu->addSeparator();
  d_add_talk_action=d_main_menu->
    addAction(tr("Add Talk Markers"),this,SLOT(addTalkData()));
  d_add_segue_action=d_main_menu->
    addAction(tr("Add Segue Markers"),this,SLOT(addSegueData()));
  d_add_hook_action=d_main_menu->
    addAction(tr("Add Hook Markers"),this,SLOT(addHookData()));
  d_add_fadeup_action=d_main_menu->
    addAction(tr("Add Fade Up Marker"),this,SLOT(addFadeupData()));
  d_add_fadedown_action=d_main_menu->
    addAction(tr("Add Fade Down Marker"),this,SLOT(addFadedownData()));
}


RDMarkerView::~RDMarkerView()
{
  if(d_scene!=NULL) {
    delete d_scene;
  }
  delete d_wave_factory;
  delete d_view;
  delete d_main_menu;
}


QSize RDMarkerView::sizeHint() const
{
  return QSize(1000,d_height);
}


QSizePolicy RDMarkerView::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
}


int RDMarkerView::audioGain() const
{
  return d_audio_gain;
}


unsigned RDMarkerView::sampleRate() const
{
  return d_sample_rate;
}


int RDMarkerView::shrinkFactor() const
{
  return d_shrink_factor;
}


int RDMarkerView::pointerValue(RDMarkerHandle::PointerRole role)
{
  return d_pointers[role];
}


RDMarkerHandle::PointerRole RDMarkerView::selectedMarker() const
{
  return d_selected_marker;
}


bool RDMarkerView::hasUnsavedChanges() const
{
  return d_has_unsaved_changes;
}


void RDMarkerView::setSelectedMarker(RDMarkerHandle::PointerRole role)
{
  if(role!=d_selected_marker) {
    for(int i=0;i<RDMarkerHandle::LastRole;i++) {
      for(int j=0;j<2;j++) {
	if(d_handles[i][j]!=NULL) {
	  d_handles[i][j]->setSelected(role==(RDMarkerHandle::PointerRole)i);
	}
      }
    }
  }
  d_selected_marker=role;
  emit selectedMarkerChanged(role);
}


void RDMarkerView::processRightClick(RDMarkerHandle::PointerRole role,
				     const QPointF &pos)
{
  d_deleting_roles.clear();

  d_deleting_roles.push_back(role);
  if((role==RDMarkerHandle::SegueStart)||
     (role==RDMarkerHandle::TalkStart)||
     (role==RDMarkerHandle::HookStart)) {
    d_deleting_roles.push_back((RDMarkerHandle::PointerRole)(role+1));
  }
  if((role==RDMarkerHandle::SegueEnd)||
     (role==RDMarkerHandle::TalkEnd)||
     (role==RDMarkerHandle::HookEnd)) {
    d_deleting_roles.push_back((RDMarkerHandle::PointerRole)(role-1));
  }
  d_marker_menu_used=true;
  d_main_menu->setGeometry(pos.x(),pos.y(),
			   d_main_menu->sizeHint().width(),
			   d_main_menu->sizeHint().height());
  d_main_menu->exec();
}


void RDMarkerView::updatePosition(RDMarkerHandle::PointerRole role, int ptr)
{
  d_pointers[role]=ptr;
  d_has_unsaved_changes=true;

  updateInterlocks();

  emit pointerValueChanged(role,d_pointers[role]);
}


void RDMarkerView::setAudioGain(int lvl)
{
  if(d_audio_gain!=lvl) {
    d_audio_gain=lvl;
    WriteWave();
  }
}


void RDMarkerView::setShrinkFactor(int sf)
{
  if((d_shrink_factor!=sf)&&(sf<=d_max_shrink_factor)) {
    d_shrink_factor=sf;
    WriteWave();
  }
}


void RDMarkerView::setMaximumShrinkFactor()
{
  setShrinkFactor(d_max_shrink_factor);
}


bool RDMarkerView::setCut(QString *err_msg,unsigned cartnum,int cutnum)
{
  clear();

  d_cart_number=cartnum;
  d_cut_number=cutnum;
  d_right_margin=LEFT_MARGIN;  // Default value
  if(!LoadCutData()) {
    *err_msg=tr("No such cart/cut!");
    return false;
  }
  if(!d_wave_factory->setCut(err_msg,cartnum,cutnum)) {
    return false;
  }
  d_audio_length=(int)((int64_t)d_wave_factory->energySize()*1152000/
		       ((int64_t)d_sample_rate));
  d_max_shrink_factor=1;
  while(d_wave_factory->energySize()>=(d_width*d_max_shrink_factor)) {
    d_max_shrink_factor=d_max_shrink_factor*2;
  }
  d_pad_size=(d_width*d_max_shrink_factor-d_wave_factory->energySize())/d_max_shrink_factor-1;
  d_shrink_factor=d_max_shrink_factor;
  WriteWave();

  updateInterlocks();

  return true;
}


void RDMarkerView::setCursorPosition(unsigned msec)
{
  qreal x=(qreal)msec*(qreal)d_sample_rate/(1152000*(qreal)d_shrink_factor)+
    (qreal)LEFT_MARGIN;
  d_cursor->setPos(x,d_cursor->pos().y());
}


void RDMarkerView::save()
{
  QString sql;

  sql=QString("update CUTS set ");
  for(int i=0;i<RDMarkerHandle::PointerRole::LastRole;i++) {
    sql+=d_pointer_fields.at(i)+QString().sprintf("=%d,",d_pointers[i]);
  }
  sql=sql.left(sql.length()-1)+" where "+
    "CUT_NAME=\""+RDEscapeString(RDCut::cutName(d_cart_number,d_cut_number))+
    "\"";
  RDSqlQuery::apply(sql);
  d_has_unsaved_changes=false;
}


void RDMarkerView::clear()
{
  d_wheel_angle=0;
  if(d_scene!=NULL) {
    delete d_scene;
    d_scene=NULL;
  }
  for(int i=0;i<RDMarkerHandle::LastRole;i++) {
    d_pointers[i]=-1;
    for(int j=0;j<2;j++) {
      d_handles[i][j]=NULL;
    }
  }
  d_cursor=NULL;
  d_shrink_factor=1;
  d_max_shrink_factor=1;
  d_pad_size=0;
  d_audio_gain=900;
  d_audio_length=0;
  d_has_unsaved_changes=false;
  d_marker_menu_used=false;
  d_selected_marker=RDMarkerHandle::LastRole;
}


void RDMarkerView::updateMenuData()
{
  bool can_add=
    (d_deleting_roles.size()==0)&&
    (Msec(d_mouse_pos)>=d_pointers[RDMarkerHandle::CutStart])&&
    (Msec(d_mouse_pos)<d_pointers[RDMarkerHandle::CutEnd]);
  bool can_delete=
    d_marker_menu_used&&
    (!d_deleting_roles.contains(RDMarkerHandle::CutStart))&&
    (!d_deleting_roles.contains(RDMarkerHandle::CutEnd));

  d_delete_marker_action->setEnabled(can_delete);
  d_add_fadedown_action->
    setEnabled(can_add&&(d_pointers[RDMarkerHandle::FadeDown]<0)&&
	       ((d_pointers[RDMarkerHandle::FadeUp]<0)||
		(d_pointers[RDMarkerHandle::FadeUp]<Msec(d_mouse_pos))));
  d_add_fadeup_action->
    setEnabled(can_add&&(d_pointers[RDMarkerHandle::FadeUp]<0)&&
	       ((d_pointers[RDMarkerHandle::FadeDown]<0)||
		(d_pointers[RDMarkerHandle::FadeDown]>Msec(d_mouse_pos))));
  d_add_hook_action->
    setEnabled(can_add&&(d_pointers[RDMarkerHandle::HookStart]<0));
  d_add_segue_action->
    setEnabled(can_add&&(d_pointers[RDMarkerHandle::SegueStart]<0));
  d_add_talk_action->
    setEnabled(can_add&&(d_pointers[RDMarkerHandle::TalkStart]<0));
}


void RDMarkerView::addTalkData()
{
  d_pointers[RDMarkerHandle::TalkStart]=Msec(d_mouse_pos);
  d_pointers[RDMarkerHandle::TalkEnd]=Msec(d_mouse_pos);

  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::TalkStart,60);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::TalkEnd,60);
  InterlockMarkerPair(RDMarkerHandle::TalkStart);

  d_has_unsaved_changes=true;

  emit pointerValueChanged(RDMarkerHandle::TalkStart,
			   d_pointers[RDMarkerHandle::TalkStart]);
  emit pointerValueChanged(RDMarkerHandle::TalkEnd,
			   d_pointers[RDMarkerHandle::TalkEnd]);
}


void RDMarkerView::addSegueData()
{
  d_pointers[RDMarkerHandle::SegueStart]=Msec(d_mouse_pos);
  d_pointers[RDMarkerHandle::SegueEnd]=Msec(d_mouse_pos);

  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::SegueStart,40);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::SegueEnd,40);
  InterlockMarkerPair(RDMarkerHandle::SegueStart);

  d_has_unsaved_changes=true;

  emit pointerValueChanged(RDMarkerHandle::SegueStart,
			   d_pointers[RDMarkerHandle::SegueStart]);
  emit pointerValueChanged(RDMarkerHandle::SegueEnd,
			   d_pointers[RDMarkerHandle::SegueEnd]);
}


void RDMarkerView::addHookData()
{
  d_pointers[RDMarkerHandle::HookStart]=Msec(d_mouse_pos);
  d_pointers[RDMarkerHandle::HookEnd]=Msec(d_mouse_pos);

  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::HookStart,100);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::HookEnd,100);
  InterlockMarkerPair(RDMarkerHandle::HookStart);

  d_has_unsaved_changes=true;

  emit pointerValueChanged(RDMarkerHandle::HookStart,
			   d_pointers[RDMarkerHandle::HookStart]);
  emit pointerValueChanged(RDMarkerHandle::HookEnd,
			   d_pointers[RDMarkerHandle::HookEnd]);
}


void RDMarkerView::addFadeupData()
{
  d_pointers[RDMarkerHandle::FadeUp]=Msec(d_mouse_pos);

  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::FadeUp,80);
  InterlockFadeMarkerPair();

  d_has_unsaved_changes=true;

  emit pointerValueChanged(RDMarkerHandle::FadeUp,
			   d_pointers[RDMarkerHandle::FadeUp]);
}


void RDMarkerView::addFadedownData()
{
  d_pointers[RDMarkerHandle::FadeDown]=Msec(d_mouse_pos);

  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::FadeDown,80);
  InterlockFadeMarkerPair();

  d_has_unsaved_changes=true;

  emit pointerValueChanged(RDMarkerHandle::FadeDown,
			   d_pointers[RDMarkerHandle::FadeDown]);
}


void RDMarkerView::deleteMarkerData()
{
  for(int i=0;i<d_deleting_roles.size();i++) {
    RemoveMarker(d_deleting_roles.at(i));
  }
  d_deleting_roles.clear();

  d_has_unsaved_changes=true;
}


void RDMarkerView::updateInterlocks()
{
  //
  // Check for "swiped" markers and remove them
  //
  for(int i=2;i<(RDMarkerHandle::LastRole);i++) {
    RDMarkerHandle::PointerRole role=(RDMarkerHandle::PointerRole)i;
    if((d_pointers[i]>=0)&&
       ((d_pointers[i]<d_pointers[RDMarkerHandle::CutStart])||
	(d_pointers[i]>d_pointers[RDMarkerHandle::CutEnd]))) {
      RemoveMarker(role);
      switch(role) {
      case RDMarkerHandle::TalkStart:
      case RDMarkerHandle::SegueStart:
      case RDMarkerHandle::HookStart:
	RemoveMarker((RDMarkerHandle::PointerRole)(i+1));
	break;

      case RDMarkerHandle::TalkEnd:
      case RDMarkerHandle::SegueEnd:
      case RDMarkerHandle::HookEnd:
	RemoveMarker((RDMarkerHandle::PointerRole)(i-1));
	break;

      case RDMarkerHandle::CutStart:
      case RDMarkerHandle::CutEnd:
      case RDMarkerHandle::FadeUp:
      case RDMarkerHandle::FadeDown:
      case RDMarkerHandle::LastRole:
	break;
      }
    }
  }

  //
  // Update the limit stops
  //
  for(int i=0;i<2;i++) {
    d_handles[RDMarkerHandle::CutStart][i]->setMinimum(0,0);
    d_handles[RDMarkerHandle::CutStart][i]->
      setMaximum(d_handles[RDMarkerHandle::CutEnd][i]->pos().x()-LEFT_MARGIN,
		 d_pointers[RDMarkerHandle::CutEnd]);

    d_handles[RDMarkerHandle::CutEnd][i]->
      setMinimum(d_handles[RDMarkerHandle::CutStart][i]->pos().x()-LEFT_MARGIN,
		 d_pointers[RDMarkerHandle::CutStart]);
    d_handles[RDMarkerHandle::CutEnd][i]->
      setMaximum(d_right_margin-LEFT_MARGIN,d_audio_length);
  }
  InterlockMarkerPair(RDMarkerHandle::TalkStart);
  InterlockMarkerPair(RDMarkerHandle::SegueStart);
  InterlockMarkerPair(RDMarkerHandle::HookStart);
  InterlockFadeMarkerPair();
}


void RDMarkerView::InterlockFadeMarkerPair()
{
  //
  // FadeUp
  //
  for(int i=0;i<2;i++) {
    if(d_handles[RDMarkerHandle::FadeUp][i]!=NULL) {
      d_handles[RDMarkerHandle::FadeUp][i]->
	setMinimum(d_handles[RDMarkerHandle::CutStart][i]->pos().x()-LEFT_MARGIN,
		   d_pointers[RDMarkerHandle::CutStart]);
      if(d_handles[RDMarkerHandle::FadeDown][i]!=NULL) {
	d_handles[RDMarkerHandle::FadeUp][i]->
	  setMaximum(d_handles[RDMarkerHandle::FadeDown][i]->pos().x()-LEFT_MARGIN,
		   d_pointers[RDMarkerHandle::FadeDown]);
      }
      else {
	d_handles[RDMarkerHandle::FadeUp][i]->
	  setMaximum(d_handles[RDMarkerHandle::CutEnd][i]->pos().x()-LEFT_MARGIN,
		     d_pointers[RDMarkerHandle::CutEnd]);	  
      }
    }
  }

  //
  // FadeDown
  //
  for(int i=0;i<2;i++) {
    if(d_handles[RDMarkerHandle::FadeDown][i]!=NULL) {
      d_handles[RDMarkerHandle::FadeDown][i]->
	setMaximum(d_handles[RDMarkerHandle::CutEnd][i]->pos().x()-LEFT_MARGIN,
		   d_pointers[RDMarkerHandle::CutEnd]);
      if(d_handles[RDMarkerHandle::FadeUp][i]!=NULL) {
	d_handles[RDMarkerHandle::FadeDown][i]->
	  setMinimum(d_handles[RDMarkerHandle::FadeUp][i]->pos().x()-LEFT_MARGIN,
		   d_pointers[RDMarkerHandle::FadeUp]);
      }
      else {
	d_handles[RDMarkerHandle::FadeDown][i]->
	  setMinimum(d_handles[RDMarkerHandle::CutStart][i]->pos().x()-LEFT_MARGIN,
		     d_pointers[RDMarkerHandle::CutStart]);	  
      }
    }
  }
}


void RDMarkerView::InterlockMarkerPair(RDMarkerHandle::PointerRole start_marker)
{
  for(int i=0;i<2;i++) {
    if(d_handles[start_marker][i]!=NULL) {
      d_handles[start_marker][i]->
	setMinimum(d_handles[RDMarkerHandle::CutStart][i]->
		   pos().x()-LEFT_MARGIN,d_pointers[RDMarkerHandle::CutStart]);
      if(d_handles[start_marker+1][i]==NULL) {
	d_handles[start_marker][i]->
	  setMaximum(d_handles[RDMarkerHandle::CutEnd][i]->
		     pos().x()-LEFT_MARGIN,d_pointers[RDMarkerHandle::CutEnd]);
      }
      else {
	d_handles[start_marker][i]->
	  setMaximum(d_handles[start_marker+1][i]->
		     pos().x()-LEFT_MARGIN,d_pointers[start_marker+1]);
      }
    }

    if(d_handles[start_marker+1][i]!=NULL) {
      if(d_handles[start_marker][i]==NULL) {
	d_handles[start_marker+1][i]->
	  setMinimum(d_handles[RDMarkerHandle::CutStart][i]->
		     pos().x()-LEFT_MARGIN,d_pointers[RDMarkerHandle::CutStart]);
      }
      else {
	d_handles[start_marker+1][i]->
	  setMinimum(d_handles[start_marker][i]->
		     pos().x()-LEFT_MARGIN,d_pointers[start_marker]);
      }
      d_handles[start_marker+1][i]->
	setMaximum(d_handles[RDMarkerHandle::CutEnd][i]->
		   pos().x()-LEFT_MARGIN,d_pointers[RDMarkerHandle::CutEnd]);
    }
  }
}




void RDMarkerView::resizeEvent(QResizeEvent *e)
{
  d_view->setGeometry(0,0,size().width(),size().height());
}


void RDMarkerView::mousePressEvent(QMouseEvent *e)
{
  int origin=0;
  int msec=0;

  if((e->x()<=LEFT_MARGIN)||(e->x()>d_right_margin)) {
    QWidget::mousePressEvent(e);
    return;
  }
  d_mouse_pos=e->x()-LEFT_MARGIN;

  if(d_marker_menu_used) {
    d_marker_menu_used=false;
    return;
  }

  switch(e->button()) {
  case Qt::LeftButton:
    if(d_view->horizontalScrollBar()!=NULL) {
      origin=d_view->horizontalScrollBar()->value();
    }
    msec=(int64_t)(d_mouse_pos+origin)*(int64_t)d_shrink_factor*1152000/
      (int64_t)d_sample_rate;
    emit positionClicked(msec);
    break;

  case Qt::MidButton:
    break;

  case Qt::RightButton:
    d_deleting_roles.clear();

    d_main_menu->setGeometry(e->globalX(),e->globalY(),
			d_main_menu->sizeHint().width(),
			d_main_menu->sizeHint().height());
    d_main_menu->exec();
    break;

  default:
    break;
  }
}


void RDMarkerView::wheelEvent(QWheelEvent *e)
{
  d_wheel_angle+=e->angleDelta().y();
  //  printf("Angle: %d\n",d_wheel_angle);

  if(d_wheel_angle>=360) {
    if(shrinkFactor()>1) {
      setShrinkFactor(shrinkFactor()/2);
    }
    d_wheel_angle=0;
  }
  if(d_wheel_angle<=-360) {
    if(shrinkFactor()<d_max_shrink_factor) {
      setShrinkFactor(shrinkFactor()*2);
    }
    d_wheel_angle=0;
  }
}


int RDMarkerView::Frame(int msec) const
{
  return (int)((int64_t)msec*(int64_t)d_sample_rate/(d_shrink_factor*1152000));
}


int RDMarkerView::Msec(int frame) const
{
  return (int)((int64_t)frame*d_shrink_factor*1152000/(int64_t)d_sample_rate);
}


bool RDMarkerView::LoadCutData()
{
  QString sql;
  RDSqlQuery *q=NULL;
  bool ret=false;

  sql=QString("select ")+
    d_pointer_fields.join(",")+","+  // 00 - 09
    "CHANNELS "+                     // 10
    "from CUTS where "+
    "CUT_NAME=\""+
    RDEscapeString(RDCut::cutName(d_cart_number,d_cut_number))+"\"";
  q=new RDSqlQuery(sql);
  if(q->first()) {
    ret=true;
    for(int i=0;i<RDMarkerHandle::LastRole;i++) {
      d_pointers[i]=q->value(i).toInt();
    }
    d_channels=q->value(10).toInt();
  }
  delete q;

  return ret;
}


void RDMarkerView::WriteWave()
{
  //
  // Waveform
  //
  QPixmap wavemap=
    d_wave_factory->generate(d_height-20,d_shrink_factor,d_audio_gain,true);

  if(d_scene!=NULL) {
    d_scene->deleteLater();
  }
  d_scene=new QGraphicsScene(0,0,wavemap.width()+d_pad_size,d_height-20,this);

  d_scene->addRect(0,0,LEFT_MARGIN,d_height-20,QPen(Qt::gray),QBrush(Qt::gray));
  d_scene->addPixmap(wavemap)->setPos(LEFT_MARGIN,0);
  d_scene->addRect(LEFT_MARGIN+wavemap.width(),0,d_pad_size,d_height-20,QPen(Qt::gray),QBrush(Qt::gray));
  d_right_margin=LEFT_MARGIN+wavemap.width();

  //
  // Markers
  //
  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::FadeDown,100);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::FadeUp,100);
  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::HookStart,80);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::HookEnd,80);
  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::TalkStart,60);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::TalkEnd,60);
  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::SegueStart,40);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::SegueEnd,40);
  DrawMarker(RDMarkerHandle::Start,RDMarkerHandle::CutStart,20);
  DrawMarker(RDMarkerHandle::End,RDMarkerHandle::CutEnd,20);

  d_cursor=new QGraphicsLineItem(0,0,0,d_height);
  d_cursor->setPen(QPen(Qt::black));
  d_scene->addItem(d_cursor);

  d_view->setScene(d_scene);

  updateInterlocks();
}


void RDMarkerView::DrawMarker(RDMarkerHandle::PointerType type,
			      RDMarkerHandle::PointerRole role,int handle_pos)
{
  QGraphicsLineItem *l_item=NULL;
  RDMarkerHandle *m_item=NULL;

  if(d_pointers[role]>=0) {
    l_item=new QGraphicsLineItem(0,0,0,d_height);
    l_item->setPen(QPen(RDMarkerHandle::pointerRoleColor(role)));
    l_item->setToolTip(RDMarkerHandle::pointerRoleTypeText(role));
    d_scene->addItem(l_item);
    l_item->setPos(LEFT_MARGIN+Frame(d_pointers[role]),0);

    m_item=new RDMarkerHandle(role,type,this);
    d_scene->addItem(m_item);
    m_item->setPos(LEFT_MARGIN+Frame(d_pointers[role]),handle_pos-12);
    d_handles[role][0]=m_item;
    m_item->setSelected(d_selected_marker==role);

    m_item=new RDMarkerHandle(role,type,this);
    d_scene->addItem(m_item);
    m_item->setPos(LEFT_MARGIN+Frame(d_pointers[role]),d_height-handle_pos-8);
    d_handles[role][1]=m_item;
    m_item->setSelected(d_selected_marker==role);
  }
}


void RDMarkerView::RemoveMarker(RDMarkerHandle::PointerRole role)
{
  d_pointers[role]=-1;
  QList<QGraphicsItem *> items=d_scene->items();
  for(int i=items.size()-1;i>=0;i--) {
    QGraphicsItem *item=items.at(i);
    if(item->toolTip().contains(RDMarkerHandle::pointerRoleText(role))) {
      d_scene->removeItem(item);
      delete item;
    }
  }
  for(int i=0;i<2;i++) {
    d_handles[role][i]=NULL;
  }

  emit pointerValueChanged(role,-1);
}
