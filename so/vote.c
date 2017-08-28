/*-------------------------------------------------------*/
/* vote.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : boards' vote routines		 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
/* brd/_/.VCH  : Vote Control Header    �ثe�Ҧ��벼���� */
/* brd/_/@vote : vote history           �L�h���벼���v	 */
/* brd/_/@/@_  : vote description       �벼����	 */
/* brd/_/@/I_  : vote selection Items   �벼�ﶵ	 */
/* brd/_/@/O_  : users' Opinions        �ϥΪ̦��ܭn��	 */
/* brd/_/@/L_  : can vote List          �i�벼�W��	 */
/* brd/_/@/G_  : voted id loG file      �w�Ⲽ�W��	 */
/* brd/_/@/Z_  : final/temporary result	�벼���G	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];


static char *
vch_fpath(fpath, folder, vch)
  char *fpath, *folder;
  VCH *vch;
{
  /* VCH �M HDR �� xname ���ǰt�A�ҥH�����ɥ� hdr_fpath() */
  hdr_fpath(fpath, folder, (HDR *) vch);
  return strrchr(fpath, '@');
}


static int vote_add();


int
vote_result(xo)
  XO *xo;
{
  char fpath[64];

  setdirpath(fpath, xo->dir, "@/@vote");
  /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
  if (more(fpath, NULL) >= 0)
    return XO_HEAD;	/* XZ_POST �M XZ_VOTE �@�� vote_result() */

  vmsg("�ثe�S������}�������G");
  return XO_FOOT;
}


static void
vote_item(num, vch)
  int num;
  VCH *vch;
{
  prints("%6d%c%c%c%c%c %-9.8s%-12s %.44s\n",
    num, tag_char(vch->chrono), vch->vgamble, vch->vsort, vch->vpercent, vch->vprivate, 
    vch->cdate, vch->owner, vch->title);
}


static int
vote_body(xo)
  XO *xo;
{
  VCH *vch;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (bbstate & STAT_BOARD)
    {
      if (vans("�n�|��벼��(Y/N)�H[N] ") == 'y')
	return vote_add(xo);
    }
    else
    {
      vmsg("�ثe�õL�벼�|��");
    }
    return XO_QUIT;
  }

  vch = (VCH *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    vote_item(++num, vch++);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
}


static int
vote_head(xo)
  XO *xo;
{
  vs_head(currBM, "�벼��");
  prints(NECKER_VOTE, d_cols, "");
  return vote_body(xo);
}


static int
vote_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(VCH));
  return vote_head(xo);
}


static int
vote_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(VCH));
  return vote_body(xo);
}


static void
vch_edit(vch, item, echo)
  VCH *vch;
  int item;		/* vlist ���X�� */
  int echo;
{
  int num, row;
  char ans[8], buf[80];

  clear();

  row = 3;

  if (echo == DOECHO)	/* �u���s�W�ɤ~��M�w�O�_����L */
    vch->vgamble = (vget(++row, 0, "�O�_����L(Y/N)�H[N] ", ans, 3, LCECHO) == 'y') ? '$' : ' ';

  if (vch->vgamble == ' ')
  {
    sprintf(buf, "�аݨC�H�̦h�i��X���H([1]��%d)�G", item);
    vget(++row, 0, buf, ans, 3, DOECHO);
    num = atoi(ans);
    if (num < 1)
      num = 1;
    else if (num > item)
      num = item;
    vch->maxblt = num;
  }
  else if (echo == DOECHO)	/* �u���s�W�ɤ~����ܽ�L������ */
  {
    /* ��L�N�u���@�� */
    vch->maxblt = 1;

    vget(++row, 0, "�аݨC������h�ֻȹ��H(100��100000)�G", ans, 7, DOECHO);
    num = atoi(ans);
    if (num < 100)
      num = 100;
    else if (num > 100000)
      num = 100000;
    vch->price = num;
  }

  vget(++row, 0, "�����벼�i��X�p�� (�ܤ֤@�p��)�H[1] ", ans, 5, DOECHO);
  num = atoi(ans);
  if (num < 1)
    num = 1;
  vch->vclose = vch->chrono + num * 3600;
  str_stamp(vch->cdate, &vch->vclose);

  if (vch->vgamble == ' ')	/* ��L�@�w�ƧǡB����ܦʤ��� */
  {
    vch->vsort = (vget(++row, 0, "�}�����G�O�_�Ƨ�(Y/N)�H[N] ", ans, 3, LCECHO) == 'y') ? 's' : ' ';
    vch->vpercent = (vget(++row, 0, "�}�����G�O�_��ܦʤ����(Y/N)�H[N] ", ans, 3, LCECHO) == 'y') ? '%' : ' ';
  }
  else
  {
    vch->vsort = 's';
    vch->vpercent = '%';
  }

  vch->vprivate = (vget(++row, 0, "�O�_����벼�W��(Y/N)�H[N] ", ans, 3, LCECHO) == 'y') ? ')' : ' ';

  if (vch->vprivate == ' ' && vget(++row, 0, "�O�_����벼���(Y/N)�H[N] ", ans, 3, LCECHO) == 'y')
  {
    vget(++row, 0, "�аݭn�n�J�X���H�W�~�i�H�ѥ[�����벼�H([0]��9999)�G", ans, 5, DOECHO);
    num = atoi(ans);
    if (num < 0)
      num = 0;
    vch->limitlogins = num;

    vget(++row, 0, "�аݭn�o��X���H�W�~�i�H�ѥ[�����벼�H([0]��9999)�G", ans, 5, DOECHO);
    num = atoi(ans);
    if (num < 0)
      num = 0;
    vch->limitposts = num;
  }
}


static int
vlist_edit(vlist)
  vitem_t vlist[];
{
  int item;
  char buf[80];

  clear();

  outs("�Ш̧ǿ�J�ﶵ (�̦h 32 ��)�A�� ENTER �����G");

  strcpy(buf, " ) ");
  for (;;)
  {
    item = 0;
    for (;;)
    {
      buf[0] = radix32[item];
      if (!vget((item & 15) + 3, (item / 16) * 40, buf, vlist[item], sizeof(vitem_t), GCARRY) || 
        (++item >= MAX_CHOICES))
	break;
    }
    if (item && vans("�O�_���s��J�ﶵ(Y/N)�H[N] ") != 'y')
      break;
  }
  return item;
}


static int
vlog_seek(fpath)
  char *fpath;
{
  VLOG old;
  int fd;
  int rc = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(VLOG)) == sizeof(VLOG))
    {
      if (!strcmp(old.userid, cuser.userid))
      {
	rc = 1;
	break;
      }
    }
    close(fd);
  }
  return rc;
}


static int
vote_add(xo)
  XO *xo;
{
  VCH vch;
  int fd, item;
  char *dir, *str, fpath[64], title[TTLEN + 1];
  vitem_t vlist[MAX_CHOICES];
  BRD *brd;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  if (!vget(b_lines, 0, "���D�G", title, TTLEN + 1, DOECHO))
    return xo->max ? XO_FOOT : vote_body(xo);	/* itoc.011125: �p�G�S������벼�A�n�^�� vote_body() */
    /* return XO_FOOT; */

  dir = xo->dir;
  if ((fd = hdr_stamp(dir, 0, (HDR *) &vch, fpath)) < 0)
  {
    vmsg("�L�k�إߧ벼������");
    return XO_FOOT;
  }

  close(fd);
  vmsg("�}�l�s�� [�벼����]");
  fd = vedit(fpath, 0); /* Thor.981020: �`�N�Qtalk�����D */
  if (fd)
  {
    unlink(fpath);
    vmsg("�����벼");
    return vote_head(xo);
  }

  strcpy(vch.title, title);
  str = strrchr(fpath, '@');

  /* --------------------------------------------------- */
  /* �벼�ﶵ�� : Item					 */
  /* --------------------------------------------------- */

  memset(vlist, 0, sizeof(vlist));
  item = vlist_edit(vlist);

  *str = 'I';
  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
  {
    vmsg("�L�k�إߧ벼�ﶵ��");
    return vote_head(xo);
  }
  write(fd, vlist, item * sizeof(vitem_t));
  close(fd);

  vch_edit(&vch, item, DOECHO);

  strcpy(vch.owner, cuser.userid);

  brd = bshm->bcache + currbno;

  brd->bvote++;
  if (brd->bvote >= 0)
    brd->bvote = (vch.vgamble == '$') ? -1 : 1;
  vch.bstamp = brd->bstamp;

  rec_add(dir, &vch, sizeof(VCH));

  vmsg("�}�l�벼�F�I");
  return vote_init(xo);
}


static int
vote_edit(xo)
  XO *xo;
{
  int pos;
  VCH *vch, vxx;
  char *dir, fpath[64];

  /* Thor: for �ק�벼�ﶵ */
  int fd, item;
  vitem_t vlist[MAX_CHOICES];
  char *fname;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  dir = xo->dir;
  vch = (VCH *) xo_pool + (pos - xo->top);

  /* Thor: �ק�벼�D�D */

  vxx = *vch;

  if (!vget(b_lines, 0, "���D�G", vxx.title, TTLEN + 1, GCARRY))
    return XO_FOOT;

  fname = vch_fpath(fpath, dir, vch);
  vedit(fpath, 0);	/* Thor.981020: �`�N�Qtalk�����D  */

  /* Thor: �ק�벼�ﶵ */

  memset(vlist, 0, sizeof(vlist));
  *fname = 'I';
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    read(fd, vlist, sizeof(vlist));
    close(fd);
  }

  item = vlist_edit(vlist);

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
  {
    vmsg("�L�k�إߧ벼�ﶵ��");
    return vote_head(xo);
  }
  write(fd, vlist, item * sizeof(vitem_t));
  close(fd);

  vch_edit(&vxx, item, GCARRY);

  if (memcmp(&vxx, vch, sizeof(VCH)))
  {
    if (vans("�T�w�n�ק�o���벼��(Y/N)�H[N] ") == 'y')
    {
      *vch = vxx;
      currchrono = vch->chrono;
      rec_put(dir, vch, sizeof(VCH), pos, cmpchrono);
    }
  }

  return vote_head(xo);
}


static int
vote_query(xo)
  XO *xo;
{
  char *dir, *fname, fpath[64], buf[80];
  VCH *vch;
  int cc, pos;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  dir = xo->dir;
  vch = (VCH *) xo_pool + (pos - xo->top);

  fname = vch_fpath(fpath, dir, vch);
  more(fpath, (char *) -1);

  *fname = 'G';
  sprintf(buf, "�@�� %d �H�ѥ[�벼�A�T�w�n�N�}���ɶ����(Y/N)�H[N] ", rec_num(fpath, sizeof(VLOG)));
  if (vans(buf) == 'y')
  {
    vget(b_lines, 0, "�Ч��}���ɶ�(-n���en�p��/+m����m�p��/0����)�G", buf, 5, DOECHO);
    if (cc = atoi(buf))
    {
      vch->vclose = vch->vclose + cc * 3600;
      str_stamp(vch->cdate, &vch->vclose);
      currchrono = vch->chrono;
      rec_put(dir, vch, sizeof(VCH), pos, cmpchrono);
    }
  }

  return vote_head(xo); 
}


static int
vfyvch(vch, pos)
  VCH *vch;
  int pos;
{
  return Tagger(vch->chrono, pos, TAG_NIN);
}


static void
delvch(xo, vch)
  XO *xo;
  VCH *vch;
{
  int fd;
  char fpath[64], buf[64], *fname;
  char *list = "@IOLGZ";	/* itoc.����: �M vote file */
  VLOG vlog;
  PAYCHECK paycheck;

  fname = vch_fpath(fpath, xo->dir, vch);

  if (vch->vgamble == '$')	/* itoc.050313: �p�G�O��L�Q�R���A����n�h��� */
  {
    *fname = 'G';

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      memset(&paycheck, 0, sizeof(PAYCHECK));
      time(&paycheck.tissue);
      sprintf(paycheck.reason, "[�h��] %s", currboard);

      while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
      {
	paycheck.money = vlog.numvotes * vch->price;
	usr_fpath(buf, vlog.userid, FN_PAYCHECK);
	rec_add(buf, &paycheck, sizeof(PAYCHECK));
      }
    }
    close(fd);
  }

  while (*fname = *list++)
    unlink(fpath); /* Thor: �T�w�W�r�N�� */
}



static int
vote_delete(xo)
  XO *xo;
{
  int pos;
  VCH *vch;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  vch = (VCH *) xo_pool + (pos - xo->top);

  if (vans(msg_del_ny) == 'y')
  {
    delvch(xo, vch);

    currchrono = vch->chrono;
    rec_del(xo->dir, sizeof(VCH), pos, cmpchrono);    
    return vote_load(xo);
  }

  return XO_FOOT;
}


static int
vote_rangedel(xo)
  XO *xo;
{
  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  return xo_rangedel(xo, sizeof(VCH), NULL, delvch);
}


static int
vote_prune(xo)
  XO *xo;
{
  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  return xo_prune(xo, sizeof(VCH), vfyvch, delvch);
}


static int
vote_pal(xo)		/* itoc.020117: �s�譭��벼�W�� */
  XO *xo;
{
  char *fname, fpath[64];
  VCH *vch;
  XO *xt;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  vch = (VCH *) xo_pool + (xo->pos - xo->top);

  if (vch->vprivate != ')')
    return XO_NONE;

  fname = vch_fpath(fpath, xo->dir, vch);
  *fname = 'L';

  xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
  xt->key = PALTYPE_VOTE;
  xover(XZ_PAL);		/* Thor: �ixover�e, pal_xo �@�w�n ready */

  free(xt);
  return vote_init(xo);
}


static int
vote_join(xo)
  XO *xo;
{
  VCH *vch, vbuf;
  VLOG vlog;
  int count, fd;
  usint choice;
  char *dir, *fname, fpath[64], buf[80], ans[4], *slist[MAX_CHOICES];
  vitem_t vlist[MAX_CHOICES];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XO_FOOT;
  }

  vch = (VCH *) xo_pool + (xo->pos - xo->top);

  /* --------------------------------------------------- */
  /* �ˬd�O�_�w�g�����벼				 */
  /* --------------------------------------------------- */

  if (time(0) > vch->vclose)
  {
    vmsg("�벼�w�g�I��F�A���R�Զ}��");
    return XO_FOOT;
  }

  /* --------------------------------------------------- */
  /* �ˬd�O�_��������					 */
  /* --------------------------------------------------- */

  if (vch->vgamble == '$')
  {
    if (cuser.money < vch->price)
    {
      vmsg("�z���������ѥ[��L");
      return XO_FOOT;
    }
  }

  /* --------------------------------------------------- */
  /* �벼�ɮ�						 */
  /* --------------------------------------------------- */

  dir = xo->dir;
  fname = vch_fpath(fpath, dir, vch);

  /* --------------------------------------------------- */
  /* �ˬd�O�_�w�g��L��					 */
  /* --------------------------------------------------- */

  if (vch->vgamble == ' ')	/* itoc.031101: ��L�i�H�@���U�` */
  {
    *fname = 'G';
    if (vlog_seek(fpath))
    {
      vmsg("�z�w�g��L���F�I");
      return XO_FOOT;
    }
  }

  /* --------------------------------------------------- */
  /* �ˬd�벼����					 */
  /* --------------------------------------------------- */

  if (vch->vprivate == ' ')
  {
    if (cuser.numlogins < vch->limitlogins || cuser.numposts < vch->limitposts)
    {
      vmsg("�z������`��I");
      return XO_FOOT;
    }
  }
  else		/* itoc.020117: �p�H�벼�ˬd�O�_�b�벼�W�椤 */
  {
    *fname = 'L';

    if (!pal_find(fpath, cuser.userno) &&
      !(bbstate & STAT_BOARD))		/* �ѩ�ä����ۤv�[�J�B�ͦW��A�ҥH�n�h�ˬd�O�_���O�D */
    {
      vmsg("�z�S�����ܥ����p�H�벼�I");
      return XO_FOOT;
    }
  }

  /* --------------------------------------------------- */
  /* �T�{�i�J�벼					 */
  /* --------------------------------------------------- */

  if (vans(vch->vgamble == ' ' ? "�O�_�ѥ[�벼(Y/N)�H[N] " : "�O�_�ѥ[��L(Y/N)�H[N] ") != 'y')
    return XO_FOOT;

  /* --------------------------------------------------- */
  /* �}�l�벼�A��ܧ벼����				 */
  /* --------------------------------------------------- */

  *fname = '@';
  more(fpath, NULL);

  /* --------------------------------------------------- */
  /* ���J�벼�ﶵ��					 */
  /* --------------------------------------------------- */

  *fname = 'I';
  if ((fd = open(fpath, O_RDONLY)) < 0)
  {
    vmsg("�L�kŪ���벼�ﶵ��");
    return vote_head(xo);
  }
  count = read(fd, vlist, sizeof(vlist)) / sizeof(vitem_t);
  close(fd);

  for (fd = 0; fd < count; fd++)
    slist[fd] = (char *) &vlist[fd];

  /* --------------------------------------------------- */
  /* �i��벼						 */
  /* --------------------------------------------------- */

  choice = 0;
  sprintf(buf, "��U���t�� %d ��", vch->maxblt); /* Thor: ��̦ܳh�X�� */
  vs_bar(buf);
  outs("�벼�D�D�G");
  for (;;)
  {
    choice = bitset(choice, count, vch->maxblt, vch->title, slist);

    if (vch->vgamble == ' ')		/* �@��벼�~��g�N�� */
      vget(b_lines - 1, 0, "�ڦ��ܭn���G", buf, 60, DOECHO);

    fd = vans("�벼 (Y)�T�w (N)���� (Q)�����H[N] ");

    if (fd == 'q')
      return vote_head(xo);

    if ((fd == 'y') && (vch->vgamble == ' ' || choice))	/* �Y�O��L�h�@�w�n�� */
      break;
  }

  /* --------------------------------------------------- */
  /* �O�����G�G�@���]���몺���p ==> �۷����o��	 */
  /* --------------------------------------------------- */

  if (vch->vgamble == '$')
  {
    /* ��L�i�H�R�J�h�i */
    for (;;)
    {
      sprintf(buf, "�C�i�䲼 %d �ȹ��A�аݭn�R�X�i�H[1] ", vch->price);
      vget(b_lines, 0, buf, ans, 3, DOECHO);	/* �̦h�R 99 �i�A�קK���� */

      if (time(0) > vch->vclose)	/* �]������ vget�A�ҥH�٭n�A�ˬd�@�� */
      {
	vmsg("�벼�w�g�I��F�A���R�Զ}��");
	return vote_head(xo);
      }

      if ((count = atoi(ans)) < 1)
	count = 1;
      fd = count * vch->price;
      if (cuser.money >= fd)
	break;
    }
  }
  else
  {
    /* �@��벼�N�O�@�i�� */
    count = 1;
  }

  /* �T�w�벼�|���I�� */
  /* itoc.050514: �]���O�D�i�H���ܶ}���ɶ��A���F�קK�ϥΪ̷|�t�b vget() �άO
     �Q�� xo_pool[] ���P�B�ӳW�� time(0) > vclose ���ˬd�A�ҥH�N�o���s���J VCH */
  if (rec_get(dir, &vbuf, sizeof(VCH), xo->pos) || vch->chrono != vch->chrono || time(0) > vbuf.vclose)
  {
    vmsg("�벼�w�g�I��F�A���R�Զ}��");
    return vote_init(xo);
  }

  if (vch->vgamble == '$')
  {
    cuser.money -= fd;	/* fd �O�n�I����� */
  }
  else if (*buf)	/* �@��벼�~��g�J�ϥΪ̷N�� */
  {
    FILE *fp;

    *fname = 'O';
    if (fp = fopen(fpath, "a"))
    {
      fprintf(fp, "�E%-12s�G%s\n", cuser.userid, buf);
      fclose(fp);
    }
  }

  /* �[�J�O���� */
  memset(&vlog, 0, sizeof(VLOG));
  strcpy(vlog.userid, cuser.userid);
  vlog.numvotes = count;
  vlog.choice = choice;
  *fname = 'G';
  rec_add(fpath, &vlog, sizeof(VLOG));

  vmsg("�벼�����I");
  return vote_head(xo);
}


struct Tchoice
{
  int count;
  vitem_t vitem;
};


static int
TchoiceCompare(i, j)
  struct Tchoice *i, *j;
{
  return j->count - i->count;
}


static char *			/* NULL:����(�٨S���H�벼) */
draw_vote(fpath, folder, vch, preview)	/* itoc.030906: �벼���G (�P account.c:draw_vote() �榡�ۦP) */
  char *fpath;
  char *folder;
  VCH *vch;
  int preview;		/* 1:�w�� 0:�}�� */
{
  struct Tchoice choice[MAX_CHOICES];
  FILE *fp;
  char *fname;
  int total, items, num, fd, ticket, bollt;
  VLOG vlog;

  fname = vch_fpath(fpath, folder, vch);

  /* vote item */

  *fname = 'I';

  items = 0;
  if (fp = fopen(fpath, "r"))
  {
    while (fread(choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
    {
      choice[items].count = 0;
      items++;
    }
    fclose(fp);
  }

  if (items == 0)
    return NULL;

  /* �֭p�벼���G */

  *fname = 'G';
  bollt = 0;		/* Thor: �`�����k�s */
  total = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
    {
      for (ticket = vlog.choice, num = 0; ticket && num < items; ticket >>= 1, num++)
      {
	if (ticket & 1)
	{
	  choice[num].count += vlog.numvotes;
	  bollt += vlog.numvotes;
	}
      }
      total++;
    }
    close(fd);
  }

  /* ���Ͷ}�����G */

  *fname = 'Z';
  if (!(fp = fopen(fpath, "w")))
    return NULL;

  fprintf(fp, "\n\033[1;34m%s\033[m\n\n"
    "\033[1;32m�� [%s] �ݪO�벼�G%s\033[m\n\n�|��O�D�G%s\n\n�|�����G%s\n\n",
    msg_seperator, currboard, vch->title, vch->owner, Btime(&vch->chrono));
  fprintf(fp, "�}������G%s\n\n\033[1;32m�� �벼�D�D�G\033[m\n\n", Btime(&vch->vclose));

  *fname = '@';
  f_suck(fp, fpath);

  fprintf(fp, "\n\033[1;32m�� �벼���G�G�C�H�i�� %d ���A�@ %d �H�ѥ[�A��X %d ��\033[m\n\n",
    vch->maxblt, total, bollt);

  if (vch->vsort == 's')
    qsort(choice, items, sizeof(struct Tchoice), TchoiceCompare);

  if (vch->vpercent == '%')
    fd = BMAX(1, bollt);
  else
    fd = 0;

  if (preview && vch->vgamble == ' ')	/* �u���w����L�~�ݭn��߲ܽv */
    preview = 0;

  for (num = 0; num < items; num++)
  {
    ticket = choice[num].count;
    if (preview)	/* ��ܥ[�R�@�i�ɪ��߲v */
      fprintf(fp, "    %-36s%5d �� (%4.1f%%) �߲v 1:%.3f\n", choice[num].vitem, ticket, 100.0 * ticket / fd, 0.9 * (bollt + 1) / (ticket + 1));
    else if (fd)
      fprintf(fp, "    %-36s%5d �� (%4.1f%%)\n", choice[num].vitem, ticket, 100.0 * ticket / fd);
    else
      fprintf(fp, "    %-36s%5d ��\n", choice[num].vitem, ticket);
  }

  /* other opinions */

  *fname = 'O';
  fputs("\n\033[1;32m�� �ڦ��ܭn���G\033[m\n\n", fp);
  f_suck(fp, fpath);
  fputs("\n", fp);
  fclose(fp);

  /* �̫�Ǧ^�� fpath �Y���벼���G�� */
  *fname = 'Z';
  return fname;
}


static int
vote_view(xo)
  XO *xo;
{
  char fpath[64];
  VCH *vch;

  vch = (VCH *) xo_pool + (xo->pos - xo->top);

  if (bbstate & STAT_BOARD || vch->vgamble == '$')
  {
    if (draw_vote(fpath, xo->dir, vch, 1))
    {
      more(fpath, NULL);
      unlink(fpath);
      return vote_head(xo);
    }

    vmsg("�ثe�|�����H�벼");
    return XO_FOOT;
  }

  return XO_NONE;
}


static void
keeplog(fnlog, board, title)
  char *fnlog;
  char *board;
  char *title;
{
  HDR hdr;
  char folder[64], fpath[64];
  FILE *fp;

  if (!dashf(fnlog))	/* Kudo.010804: �ɮ׬O�Ū��N�� keeplog */
    return;

  brd_fpath(folder, board, fn_dir);

  if (fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w"))
  {
    fprintf(fp, "�@��: %s (%s)\n���D: %s\n�ɶ�: %s\n\n",
      str_sysop, SYSOPNICK, title, Btime(&hdr.chrono));
    f_suck(fp, fnlog);
    fclose(fp);

    strcpy(hdr.title, title);
    strcpy(hdr.owner, str_sysop);
    rec_bot(folder, &hdr, sizeof(HDR));

    btime_update(brd_bno(board));
  }
}


static void
vlog_pay(fpath, choice, fp, vch)/* �߿�����諸�ϥΪ� */
  char *fpath;			/* �O���ɸ��| */
  usint choice;			/* ���T������ */
  FILE *fp;			/* �g�J���ɮ� */
  VCH *vch;
{
  int fd;
  int correct, bollt;		/* ���/���� ������ */
  int single, money;
  char buf[64];
  VLOG vlog;
  PAYCHECK paycheck;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    /* �Ĥ@���X�߲v */
    correct = bollt = 0;
    while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
    {
      bollt += vlog.numvotes;
      if (vlog.choice == choice)
	correct += vlog.numvotes;
    }

    /* ���O�D���Y 1% */
    money = (INT_MAX / vch->price) * 100;	/* BioStar.050626: �קK���� */
    money = (bollt > money) ? INT_MAX : vch->price / 100 * bollt;
    fprintf(fp, "�O�D %s ���Y�A�i��o %d �ȹ�\n", vch->owner, money);

    memset(&paycheck, 0, sizeof(PAYCHECK));
    time(&paycheck.tissue);
    paycheck.money = money;
    sprintf(paycheck.reason, "[���Y] %s", currboard);
    usr_fpath(buf, vch->owner, FN_PAYCHECK);
    rec_add(buf, &paycheck, sizeof(PAYCHECK));

    if (correct)	/* �p�G�S�H�㤤�A�N���ݭn�o�� */
    {
      /* �o�����A�t�Ω� 10% ���| */
      single = (float) vch->price * 0.9 * bollt / correct;
      fprintf(fp, "�C�i�i�� %d �ȹ��A��諸�ϥΪ̦��G\n", single);

      /* �ĤG��}�l�o�� */
      lseek(fd, (off_t) 0, SEEK_SET);
      while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
      {
	if (vlog.choice == choice)
	{
	  money = INT_MAX / single;		/* BioStar.050626: �קK���� */
	  money = (vlog.numvotes > money) ? INT_MAX : single * vlog.numvotes;
	  fprintf(fp, "%s �R�F %d �i�A�@�i��o %d �ȹ�\n", vlog.userid, vlog.numvotes, money);

	  paycheck.money = money;
	  sprintf(paycheck.reason, "[��L] %s", currboard);
	  usr_fpath(buf, vlog.userid, FN_PAYCHECK);
	  rec_add(buf, &paycheck, sizeof(PAYCHECK));
	}
      }
    }

    close(fd);
  }
}


static int
vote_open(xo)
  XO *xo;
{
  int pos, fd, count;
  char *dir, *fname, fpath[64], buf[80];
  usint choice;
  char *slist[MAX_CHOICES];
  vitem_t vlist[MAX_CHOICES];
  VCH *vch;
  FILE *fp;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  vch = (VCH *) xo_pool + (pos - xo->top);

  if (time(NULL) < vch->vclose)
  {
    if (vans("�|�����w�}���ɶ��A�T�w�n�����}��(Y/N)�H[N] ") != 'y')
      return XO_FOOT;
  }

  dir = xo->dir;

  /* �벼���G */

  if (!(fname = draw_vote(fpath, dir, vch, 0)))
  {
    vmsg("�ثe�|�����H�벼");
    return XO_FOOT;
  }

  if (vch->vgamble == '$')	/* ��L */
  {
    /* �O�D��J���G�A�üg�J�벼���G */
    while (!vget(b_lines, 0, "�п�J���T���סG", buf, 60, DOECHO))
      ;

    /* ���J�벼�ﶵ�� */
    *fname = 'I';
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      count = read(fd, vlist, sizeof(vlist)) / sizeof(vitem_t);
      close(fd);

      for (fd = 0; fd < count; fd++)
	slist[fd] = (char *) &vlist[fd];

      /* �O�D��X���T���� */
      choice = 0;
      vs_bar("��ܥ��T����");
      outs("�벼�D�D�G");
      for (;;)
      {
	choice = bitset(choice, count, vch->maxblt, vch->title, slist);

	fd = vans("�}�� (Y)�T�w (N)���� (Q)�����H[N] ");

	if (fd == 'q')
	{
	  *fname = 'Z';
	  unlink(fpath);
	  return vote_head(xo);
	}

	if (fd == 'y' && choice)	/* �Y�O��L�h�@�w�n�� */
	  break;
      }

      /* �}�l�o�� */
      *fname = 'Z';
      if (fp = fopen(fpath, "a"))
      {
	fprintf(fp, "�O�D���G���סG%s\n\n", buf);

	*fname = 'G';
	vlog_pay(fpath, choice, fp, vch);

	fputs("\n", fp);
	fclose(fp);
      }

      /* �}�����G */
      *fname = 'Z';
    }
  }

  /* �N�}�����G post �� [BN_RECORD] �P ���ݪO */

  if (!(currbattr & BRD_NOVOTE))
  {
    sprintf(buf, "[�O��] %s <<�ݪO�ﱡ����>>", currboard);
    keeplog(fpath, BN_RECORD, buf);
  }

  keeplog(fpath, currboard, "[�O��] �ﱡ����");

  /* �벼���G���[�� @vote */

  setdirpath(buf, dir, "@/@vote");
  if (fp = fopen(fpath, "a"))
  {
    f_suck(fp, buf);
    fclose(fp);
    rename(fpath, buf);
  }

  /* �}�����N�R�� */
  vch->vgamble = ' ';	/* �O���D��L�A�p���b delvch �̭��N���|�h��� */
  delvch(xo, vch);

  currchrono = vch->chrono;
  rec_del(dir, sizeof(VCH), pos, cmpchrono);    

  vmsg("�}������");
  return vote_init(xo);
}


static int
vote_tag(xo)
  XO *xo;
{
  VCH *vch;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  vch = (VCH *) xo_pool + cur;

  if (tag = Tagger(vch->chrono, pos, TAG_TOGGLE))
  {
    move(3 + cur, 6);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE;	/* lkchu.981201: ���ܤU�@�� */
}


static int
vote_help(xo)
  XO *xo;
{
  xo_help("vote");
  return vote_head(xo);
}


static KeyFunc vote_cb[] =
{
  XO_INIT, vote_init,
  XO_LOAD, vote_load,
  XO_HEAD, vote_head,
  XO_BODY, vote_body,

  'r', vote_join,	/* itoc.010901: ���k������K */
  'v', vote_join,
  'R', vote_result,

  'V', vote_view,
  'E', vote_edit,
  'o', vote_pal,
  'd', vote_delete,
  'D', vote_rangedel,
  't', vote_tag,
  'b', vote_open,

  Ctrl('D'), vote_prune,
  Ctrl('G'), vote_pal,
  Ctrl('P'), vote_add,
  Ctrl('Q'), vote_query,

  'h', vote_help
};


int
XoVote(xo)
  XO *xo;
{
  char fpath[64];

  /* �� post �v�Q���~��ѥ[�벼 */
  /* �ӥB�n�קK guest �b sysop �O�벼 */

  if (!(bbstate & STAT_POST) || !cuser.userlevel)
    return XO_NONE;

  setdirpath(fpath, xo->dir, FN_VCH);
  if (!(bbstate & STAT_BOARD) && !rec_num(fpath, sizeof(VCH)))
  {
    vmsg("�ثe�S���벼�|��");
    return XO_FOOT;
  }

  xz[XZ_VOTE - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_VOTE - XO_ZONE].cb = vote_cb;
  xover(XZ_VOTE);
  free(xo);

  return XO_INIT;
}


int
vote_all()		/* itoc.010414: �벼���� */
{
  typedef struct
  {
    char brdname[BNLEN + 1];
    char class[BCLEN + 1];
    char title[BTLEN + 1];
    char BM[BMLEN + 1];
    char bvote;
  } vbrd_t;

  extern char brd_bits[];
  char *str;
  char fpath[64];
  int num, pageno, pagemax, redraw;
  int ch, cur;
  BRD *bhead, *btail;
  XO *xo;
  vbrd_t vbrd[MAXBOARD], *vb;

  bhead = bshm->bcache;
  btail = bhead + bshm->number;
  cur = 0;
  num = 0;

  do
  {
    str = &brd_bits[cur];
    ch = *str;
    if (bhead->bvote && (ch & BRD_W_BIT))
    {
      vb = vbrd + num;
      strcpy(vb->brdname, bhead->brdname);
      strcpy(vb->class, bhead->class);
      strcpy(vb->title, bhead->title);
      strcpy(vb->BM, bhead->BM);
      vb->bvote = bhead->bvote;
      num++;
    }
    cur++;
  } while (++bhead < btail);

  if (!num)
  {
    vmsg("�ثe�����èS������벼");
    return XEASY;
  }

  num--;
  pagemax = num / XO_TALL;
  pageno = 0;
  cur = 0;
  redraw = 1;

  do
  {
    if (redraw)
    {
      /* itoc.����: �ɶq���o�� xover �榡 */
      vs_head("�벼����", str_site);
      prints(NECKER_VOTEALL, d_cols >> 1, "", d_cols - (d_cols >> 1), "");

      redraw = pageno * XO_TALL;	/* �ɥ� redraw */
      ch = BMIN(num, redraw + XO_TALL - 1);
      move(3, 0);
      do
      {
	vb = vbrd + redraw;
	/* itoc.010909: �O�W�Ӫ����R���B�[�����C��C���] BCLEN = 4 */
	prints("%6d   %-13s\033[1;3%dm%-5s\033[m%s %-*.*s %.*s\n",
	  redraw + 1, vb->brdname,
	  vb->class[3] & 7, vb->class,
	  vb->bvote > 0 ? ICON_VOTED_BRD : ICON_GAMBLED_BRD,
	  (d_cols >> 1) + 34, (d_cols >> 1) + 33, vb->title, d_cols - (d_cols >> 1) + 13, vb->BM);

	redraw++;
      } while (redraw <= ch);

      outf(FEETER_VOTEALL);
      move(3 + cur, 0);
      outc('>');
      redraw = 0;
    }

    switch (ch = vkey())
    {
    case KEY_RIGHT:
    case '\n':
    case ' ':
    case 'r':
      vb = vbrd + (cur + pageno * XO_TALL);

      /* itoc.060324: ���P�i�J�s���ݪO�AXoPost() �������ơA�o�̴X�G���n�� */
      if (!vb->brdname[0])	/* �w�R�����ݪO */
	break;

      redraw = brd_bno(vb->brdname);	/* �ɥ� redraw */
      if (currbno != redraw)
      {
	ch = brd_bits[redraw];

	/* �B�z�v�� */
	if (ch & BRD_M_BIT)
	  bbstate |= (STAT_BM | STAT_BOARD | STAT_POST);
	else if (ch & BRD_X_BIT)
	  bbstate |= (STAT_BOARD | STAT_POST);
	else if (ch & BRD_W_BIT)
	  bbstate |= STAT_POST;

	mantime_add(currbno, redraw);

	currbno = redraw;
	bhead = bshm->bcache + currbno;
	currbattr = bhead->battr;
	strcpy(currboard, bhead->brdname);
	str = bhead->BM;
	sprintf(currBM, "�O�D�G%s", *str <= ' ' ? "�x�D��" : str);
#ifdef HAVE_BRDMATE
	strcpy(cutmp->reading, currboard);
#endif

	brd_fpath(fpath, currboard, fn_dir);
#ifdef AUTO_JUMPPOST
	xz[XZ_POST - XO_ZONE].xo = xo = xo_get_post(fpath, bhead);	/* itoc.010910: �� XoPost �q�����y�@�� xo_get() */
#else
	xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
#endif
	xo->key = XZ_POST;
	xo->xyz = bhead->title;
      }

      sprintf(fpath, "brd/%s/%s", currboard, FN_VCH);
      xz[XZ_VOTE - XO_ZONE].xo = xo = xo_new(fpath);
      xz[XZ_VOTE - XO_ZONE].cb = vote_cb;
      xover(XZ_VOTE);
      free(xo);
      redraw = 1;
      break;

    default:
      ch = xo_cursor(ch, pagemax, num, &pageno, &cur, &redraw);
      break;
    }
  } while (ch != 'q');

  return 0;
}