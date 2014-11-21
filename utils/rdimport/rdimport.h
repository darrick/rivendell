// rdimport.h
//
// A Batch Importer for Rivendell.
//
//   (C) Copyright 2002-2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: rdimport.h,v 1.17.6.3.2.3 2014/07/15 00:45:17 cvs Exp $
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


#ifndef RDIMPORT_H
#define RDIMPORT_H

#include <list>
#include <vector>

#include <qobject.h>
#include <qsqldatabase.h>
#include <qfileinfo.h>
#include <qdatetime.h>

#include <rdwavedata.h>
#include <rdwavefile.h>
#include <rdconfig.h>
#include <rdcmd_switch.h>
#include <rdgroup.h>
#include <rdcart.h>
#include <rdcut.h>
#include <rdripc.h>
#include <rduser.h>
#include <rdsystem.h>
#include <rdstation.h>

#include "markerset.h"

#define RDIMPORT_TEMP_BASENAME "rdimp"
#define RDIMPORT_STDIN_BUFFER_LENGTH 1024
#define RDIMPORT_DROPBOX_SCAN_INTERVAL 5
#define RDIMPORT_DROPBOX_PASSES 3
#define RDIMPORT_USAGE "[options] <group> <filespec> [<filespec>]*\n\nAudio importation tool for the Rivendell Radio Automation System.\nDo 'man 1 rdimport' for the full manual.\n"
#define RDIMPORT_GLOB_SIZE 10

class MainObject : public QObject
{
  Q_OBJECT;
 public:
  MainObject(QObject *parent=0,const char *name=0);

 private slots:
  void userData();

 private:
  enum Result {Success=0,FileBad=1,NoCart=2,NoCut=3};
  void RunDropBox();
  void ProcessFileList(const QString &flist);
  void ProcessFileEntry(const QString &entry);
  MainObject::Result ImportFile(const QString &filename,unsigned *cartnum);
  void VerifyFile(const QString &filename,unsigned *cartnum);
  RDWaveFile *FixFile(const QString &filename,RDWaveData *wavedata);
  bool IsWav(int fd);
  bool FindChunk(int fd,const char *name,bool *fix_needed);
  bool FixChunkSizes(const QString &filename);
  bool RunPattern(const QString &pattern,const QString &filename,
		  RDWaveData *wavedata,QString *groupname);
  bool VerifyPattern(const QString &pattern);
  void PrintLogDateTime(FILE *f=stdout);
  void DeleteCuts(unsigned cartnum);
  QDateTime GetCachedTimestamp(const QString &filename);
  void WriteTimestampCache(const QString &filename,const QDateTime &dt);
  bool SchedulerCodeExists(const QString &code) const;
  RDConfig *import_config;
  RDCmdSwitch *import_cmd;
  unsigned import_file_key;
  RDGroup *import_group;
  RDRipc *import_ripc;
  RDUser *import_user;
  bool import_verbose;
  bool import_log_mode;
  bool import_use_cartchunk_cutid;
  int import_cart_number_offset;
  bool import_single_cart;
  bool import_title_from_cartchunk_cutid;
  bool import_delete_source;
  bool import_delete_cuts;
  bool import_drop_box;
  std::vector<QString> import_add_scheduler_codes;
  QString import_set_user_defined;
  bool import_stdin_specified;
  int import_startdate_offset;
  int import_enddate_offset;
  bool import_create_dates;
  int import_create_startdate_offset;
  int import_create_enddate_offset;
  QDateTime import_datetimes[2];
  bool import_clear_datetimes;
  QTime import_dayparts[2];
  bool import_clear_dayparts;
  bool import_fix_broken_formats;
  int import_persistent_dropbox_id;
  unsigned import_format;
  unsigned import_samprate;
  unsigned import_bitrate;
  unsigned import_channels;
  int import_src_converter;
  int import_normalization_level;
  int import_autotrim_level;
  int import_segue_level;
  int import_segue_length;
  unsigned import_cart_number;
  QString import_metadata_pattern;
  QString import_string_agency;
  QString import_string_album;
  QString import_string_artist;
  int import_string_bpm;
  QString import_string_client;
  QString import_string_composer;
  QString import_string_conductor;
  QString import_string_description;
  QString import_string_outcue;
  QString import_string_publisher;
  QString import_string_label;
  QString import_string_song_id;
  QString import_string_title;
  QString import_string_user_defined;
  int import_string_year;
  struct DropboxList {
    QString filename;
    unsigned size;
    unsigned pass;
    bool checked;
    bool failed;
  };
  std::list<DropboxList *> import_dropbox_list;
  QString import_temp_fix_filename;
  RDSystem *import_system;
  RDStation *import_station;
  MarkerSet *import_cut_markers;
  MarkerSet *import_talk_markers;
  MarkerSet *import_hook_markers;
  MarkerSet *import_segue_markers;
  MarkerSet *import_fadedown_marker;
  MarkerSet *import_fadeup_marker;
};


#endif  // RDIMPORT_H
