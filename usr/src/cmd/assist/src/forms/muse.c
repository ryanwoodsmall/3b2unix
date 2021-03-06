/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)forms:muse.c	1.20"

/* muse.c -- main() routine of mforms.  Does following steps:
 *  1. Command line parsing.
 *  2. Initialization of global variables and of curses stdscr window.
 *  3. Reading in of help files and popup menu file.
 *  4. Do a while() loop with read_in() [reads new fs file] and
 *     fill_out() [handles all user/form/menu interaction]
 */
#include "musedefs.h"
#include "mmusedefs.h"

#define MINFS(X) {if (strlen(X)>3 && strcmp(X+strlen(X)-3,".fs")==0)\
                  *(X+strlen(X)-3) = null;}

main(argc,argv)
int argc;
char *argv[];
{  register int i, c, test=1;
   char help_name[80];
   register char *s, *s0, **b, *c_pt;
   struct field *f_pt;
   struct segment *s_pt;
   char *termin, *getenv();
   struct stat buf;
   extern int done();

   status=12;
   signal(SIGINT,SIG_IGN);
   signal(SIGTERM,SIG_IGN);
   signal(SIGQUIT,SIG_IGN);
   signal(SIGALRM,SIG_IGN);



/* Initialize labels[] array, containing the names of all
   traversed fs files plus the selected menu items (memory) */
   lab_pt = labels;
   lab_pt->files_name = (char *)calloc(6,sizeof(char));
   strcpy(lab_pt->files_name,"UNIX");
   strcpy(lab_pt->screen_name,"UNIX");
   lab_pt->memory = 0;

/*
 * Define paths for fs files, tutorials, scripts, and mecho.
 */

   sprintf(muse,"/usr/lib/assist/");

   if ((s=(char*)getenv("ASSISTBIN")) != NULL && *s)
      sprintf(musebin,"%s/",s);
   else sprintf(musebin,"%sbin/",muse);

   if ((s=(char*)getenv("ASSISTLIB")) != NULL && *s)
      sprintf(muselib,"%s/",s);
   else sprintf(muselib,"%slib/",muse);

   
   if ((s=(char*)getenv("FORMS")) != NULL && *s)
      sprintf(forms,"%s/",s);
   else sprintf(forms,"%sforms/",muselib);

   sprintf(refs,"%srefs/",muselib);

   sprintf(tutorials,"%stutorials/",muselib);

   sprintf(mecho,"%smecho",musebin);

   if (access(mecho,01)!=0) {
      fprintf(stderr,"%s does not exist or does not have execute permission\n",mecho);
      exit(1);
      
   }
   
   strcpy(fs_forms,forms);

/* Command line parsing.  fs_forms is .fs file directory.
   next_screen stores name of next screen in labels[] */
   switch(argc)
   {
      case 1:                            /* assist */
         localflag = 0;
         next_screen("TOP",NEW);
         break;
      case 2:
         if (strcmp(argv[1],"-c")==0)    /* assist -c */
         {
            *fs_forms = '\0';
            localflag = 1;
            next_screen("TOP",NEW);
         }
         else                            /* assist <cmd> */
         {
            MINFS(argv[1]);
            if (strlen(argv[1]) > 128) 
            {
               status=13;
               done();
            }
            localflag = 0;
            next_screen(argv[1],NEW);
         }
         break;
      case 3:
         if (strcmp(argv[1],"-c")==0)    /* assist -c <cmd> */
         {
            *fs_forms = '\0';
            MINFS(argv[2]);
            if (strlen(argv[2]) > 128) 
            {
               status=13;
               done();
            }
            localflag = 1;
            next_screen(argv[2],NEW);
         }
         else
         {
            status=1;
            done();
         }
         break;
      default:
         status = 1;
         done();
         break;
   }

/* Parse $PATH into separate strings */
   test = 1;
   s0 = s = (char*)getenv("PATH");
   b = binpaths;
   if (s==NULL) *b=NULL;
   else while (test)
   {
      if (*s==':' || *s==null)
      {
         *b = (char*)calloc((unsigned)(s-s0+1),sizeof(char)); /* Also 0's *b */
         strncpy(*b++,s0,s-s0);
         *b = NULL;
         s0 = s+1;
         if (*s==null) test=0;
      }
      s++;
   }

/* Check of fs file is accessible */
   if (implement(lab_pt->files_name,1)==0) {
      fprintf(stderr,"%s\n",error_mess);
      exit(1);
   }

/* Give friendly message */
   printf("\nSTARTING UP.  PLEASE WAIT ... \n");
   fflush(stdout);

/* Parse $CDPATH into separate strings */
   test = 1;
   s0 = s = (char*)getenv("CDPATH");
   b = cdpaths;
   if (s==NULL) *b=NULL;
   else while (test)
   {
      if (*s==':' || *s==null)
      {
         *b = (char*)calloc((unsigned)(s-s0+1),sizeof(char)); /* Also 0's *b */
         strncpy(*b++,s0,s-s0);
         *b = NULL;
         s0 = s+1;
         if (*s==null) test=0;
      }
      s++;
   }

   
/* Set terminal parameters */
   term = 0;
   if ((c_pt=(char*)getenv("MSTANDOUT")) != NULL && *c_pt=='1')
      term |= STANDOUT;
   if ((c_pt=(char*)getenv("MFKEYS")) != NULL && *c_pt=='1')
      term |= FKEYS;
   if ((c_pt=(char*)getenv("MALTCHAR")) != NULL && *c_pt=='1')
      term |= ALTCHAR;
   if ((termin=(char*)getenv("TERM")) == NULL) 
      termin="vt100";

/* Define special characters for smooth lines */
   if (term & ALTCHAR)
   {
      HOR = hor = 'q';
      VER = ver = 'x';
      TL  = tl  = 'l';
      TR  = tr  = 'k';
      BL  = bl  = 'm';
      BR  = br  = 'j';
   }
   else
   {
      HOR = hor = '-';
      VER = ver = '|';
      TL  = tl  = SPACE;
      TR  = tr  = SPACE;
      BL  = bl  = SPACE;
      BR  = br  = SPACE;
   }

/* Define key labels */
   if (term & FKEYS)
   {
      strcpy(f[1],"f1");
      strcpy(f[2],"f2");
      strcpy(f[3],"f3");
      strcpy(f[4],"f4");
      strcpy(f[5],"f5");
      strcpy(f[6],"f6");
      strcpy(f[7],"f7");
      strcpy(f[8],"f8");
   }

   else
   {
      strcpy(f[1],"^G");
      strcpy(f[2],"^V");
      strcpy(f[3],"^L");
      strcpy(f[4],"^R");
      strcpy(f[5],"^F");
      strcpy(f[6],"^Y");
      strcpy(f[7],"^O");
      strcpy(f[8],"^A");
   }

/* Some very special processing for dmd-netty windows and for 5425 */
   if (equaln(termin,"netty",5)) {
      for (i=0;i<14;i++) putchar('\n');
      fkeys();  /* Download function keys */
   }
   else if (equal(termin,"5420") || equal(termin,"5425")) {
      for (i=0;i<22;i++) putchar('\n');
   }

/* Initialize ">" pointer */
   arrow.row = arrow_buff.row = 0;
   arrow.col = arrow_buff.col = 0;
   arrow.word = (char *)calloc(3,sizeof(char));
   arrow_buff.word = (char *)calloc(3,sizeof(char));
   strcpy(arrow.word,">");
   *arrow_buff.word = null;

/* Set mode to "neutral" */
   mode = NEWFIELD;
  
/* Exit status */
   status = 0;

/* Store terminal state */
   catchtty(&termbuf);

/* Get into "curses" state */
   curses(ON);


/* Check screen dimensions */
   if (LINES<24)
   {
      status = 17;
      done();
   }
   if (COLS<80)
   {
      status = 18;
      done();
   }

 /* Initialize horizonal lines */
   for (i=0;i<COLS;i++)   stripes[i] = hor;
   stripes[COLS] = null;


/* READ MENU HELP FILE  */
   /* 1: f8 help */

   sprintf(help_name,"%sg_help_m",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = menu_help = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;



   /* 2: ^A help */
   sprintf(help_name,"%sd_help_m",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = menu_help0 = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;



/* READ CF HELP FILE */
   /* 1: f8 help */

   sprintf(help_name,"%sg_help_c",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = cf_help = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;


   /* 2: ^A help */
   sprintf(help_name,"%sd_help_c",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = cf_help0 = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;

/* READ TOP HELP FILE (obsolete: old popup menu item)  */

   sprintf(help_name,"%sg_help_t",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = top_help = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;


   sprintf(help_name,"%sd_help_t",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = top_help0 = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;

/* READ POPUP MENU HELP */
   /* 1: f8 help */
   sprintf(help_name,"%sg_help_p",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = pu_help = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;

   /* 2: ^A help */
   sprintf(help_name,"%sd_help_p",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   c_pt = pu_help0 = (char*)calloc((unsigned)(buf.st_size+2),sizeof(char));
   fp=fopen(help_name,"r");
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   fclose(fp);
   *c_pt = null;



/* READ IN POPUP MENU  */
   sprintf(help_name,"%spu_menu.fs",forms);
   if (stat(help_name,&buf)!=0 && access(help_name,04)!=0) {
      endwin();
      printf("%s does not exist or has no read permission\n",help_name);
      exit(1);
   }
   fp=fopen(help_name,"r");

   mode = 0; /* 0: processing segment, 1: processing item help; 
                an misuse of "mode" */

   /* Initialize first menu field */
   f_pt = menus;
   f_pt->type = 1;
   s_pt = f_pt->first_s_pt = 
                     (struct segment *)calloc(1,sizeof(struct segment));
   s_pt->word = (char *)calloc(32,sizeof(char));
   c_pt = s_pt->word;

   /* Now read and store file 
      File format is not that of ordinary fs file:
      <selectable>^X<multi-line item help>^Z<2-digit location index>
      <selectable>^X<multi-line item help>^Z<2-digit location index>
      ...
      ^R
      <menu help>  

      The location indices are hardcoded [in menu() and fill_out()].
      They also determine the action taken when an item is selected.
   */
   while ((c=getc(fp))!=CTRL(R))
   switch(c)
   {
   case CTRL(X):   /* Terminate selectable */
      *c_pt = null;
      m_length = (strlen(s_pt->word)>m_length) ? strlen(s_pt->word) :
                   m_length;
      m_lines++;
      last_menu = f_pt;
      if (strlen(f_pt->first_s_pt->word)) f_pt->type = 1;
      else f_pt->type = 0;
      c_pt = f_pt->help = (char *)calloc(1024,sizeof(char));
      mode = 1;
      break;
   case CTRL(Z):   /* Terminate item help. Compute locations.
                      Initialize next segment to receive next selectable */
      *c_pt++ = '\n';
      *c_pt = null;
      c = getc(fp);
      f_pt->loc = 10*(c-'0');
      c = getc(fp);
      f_pt->loc += (c-'0');
      s_pt = (++f_pt)->first_s_pt = 
                (struct segment *)calloc(1,sizeof(struct segment));
      c_pt = s_pt->word = (char *)calloc(32,sizeof(char));
      mode = 0;
      break;
   case '\n':
      if (mode==1) *c_pt++=c;
      break;
   default:
      *c_pt++ = c;
      break;
   }
   m_length-=1;  /* Adjust menu box size */
   m_lines-=2;
   *c_pt  = null;

   /* Now read popup menu help */
   c=getc(fp);  /*Dummy read <CR> */
   c_pt = pu_menu_help = (char*)calloc(1024,sizeof(char));;
   while ((c=getc(fp))!=EOF)
      *c_pt++ = c;
   *c_pt = null;

   fclose(fp);
   

/*
 * Initialize for read_in().
 */

   buffer = NULL;


/* CENTRAL LOOP  */

   while (diffn(lab_pt->files_name,"UNIX",4))
   {
      read_in(1);
      fill_out();
   }
   status = 0;
   done();
}



