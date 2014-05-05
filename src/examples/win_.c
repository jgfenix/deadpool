#include <Elementary.h>
#include <unistd.h>
#define MAX 500

static void
  on_done(void *data, Evas_Object *obj, void *event_info)
  {
          elm_exit();
  }

/*static void / * hook on the sole smart callback * /
_file_chosen(void            *data,
             Evas_Object *obj,
             void            *event_info)
{
   Evas_Object *entry = data;
   const char *file = event_info;
   if (file)
     {
        elm_object_text_set(entry, file);
        printf("File chosen: %s\n", file);
     }
   else
     printf("File selection canceled.\n");
}
*/

/*static void call_getcwd ()//printa o diretorio atual de trabalho
{
    char * cwd;
    cwd = getcwd (0, 0);
    if (! cwd) {
	fprintf (stderr, "getcwd failed: %s\n", strerror (errno));
    } else {
	printf ("%s\n", cwd);
	free (cwd);
    }
}
*/

EAPI_MAIN int

elm_main(int argc, char **argv)
{
   Evas_Object *win, *vbox, *box, *sep, *en, *fs_bt, *folder, *scroll, *label, *_list, *left, *right, *b_exit;

  FILE *f;
  int i = 0;
  char c;
  char opp[MAX];
  memset(opp, MAX, '\0');
 
  chdir("/home/gabriel"); 
  
  if( (f = fopen("opp", "r")) == NULL)
    {
       perror("&&");
       exit(0);
    }

  else{  
       while(((c = getc(f)) != EOF) && i< MAX)
        {
           //if(c == '\n') c = "<br>";
           opp[i] = c;
           ++i ;
        };
      };

   
  // call_getcwd ();
     //printf("**%s", opp);
  //ver elm_general.h sobre o elm_policy_set()
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);  

    //cria uma janela,define um tamanho e exibe
    win = elm_win_util_standard_add("Player", "Player_test");
    evas_object_smart_callback_add(win, "delete,request", on_done, NULL);//adiciona o contexto de fechar a janela no 'X' no canto da janela
    elm_win_autodel_set(win, EINA_TRUE); //se a janela recebe um sinal para fechar,ela se destroi | ver elm_win_legacy.h
    evas_object_resize(win, 400, 300);//tamanho da janela
    elm_box_horizontal_set(win, EINA_TRUE);//se a janela nao estiver associada ao box,e' pode-se seta-la para não ficar no padrao horizontal
    evas_object_show(win);

    box = elm_box_add(win);// associa a janela a um box | ver elm_box_legacy.h
    elm_box_homogeneous_set(win,EINA_TRUE);
    elm_box_horizontal_set(box, EINA_TRUE);//se não setar como horizontal,o padrao e' vertical para a ordem do que e' 
                                           //adicionado ao box EINA_TRUE = 1, EINA_FALSE = 0 

    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); //habilita a janela a poder ser alterada pelo usuario em tempo de execucao |elm_win_legacy.h

    elm_win_resize_object_add(win, box);//a diferenca entre isso e evas_object_resize(box, 200, 320) e' que em elm_win_resize
                                        //a janela nao pode ser alterada posteriormente pelo usuario
    evas_object_show(box);

    vbox = elm_box_add(win);
  //  evas_win_autodel_set(vbox, EINA_TRUE);
  //  evas_object_resize(vbox, 200, 150);
    elm_box_horizontal_set(vbox, EINA_FALSE);
    evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); 
    elm_win_resize_object_add(win, vbox);
    elm_box_homogeneous_set(vbox,EINA_TRUE);
    evas_object_show(vbox);


    left = elm_button_add(box);//elm_button.h
    elm_object_text_set(left, " << ");//da' um titulo de exibicao ao botao
    elm_box_pack_end(box, left);//adiciona um botao no final da 'pilha' do box,se torna o primeiro na exibicao
    evas_object_show(left);
    
    b_exit = elm_button_add(box);
    elm_object_text_set(b_exit, " EXIT ");
    elm_box_pack_end(box, b_exit);
    evas_object_smart_callback_add(b_exit, "clicked", on_done, NULL);//resposta ao clicar em '>', a janela fecha usando a funcao on_done
    evas_object_show(b_exit);
    
    right = elm_button_add(box);
    elm_object_text_set(right, " >> ");
    elm_box_pack_end(box, right);
    evas_object_show(right);
   
   
    folder = elm_icon_add(box);//folder passa a ser um objeto tipo imagem dentro do box,se der erro,retorna NULL 
    elm_icon_order_lookup_set(folder, ELM_ICON_LOOKUP_THEME_FDO);//define a ordem de como o icone sera' procurado |ordem: freedesktop, theme | elm_icon_legacy.h
    
    elm_icon_standard_set(folder, "menu/folder");// procura e seta o icone pelo nome dele, 'menu/folder'
    evas_object_show(folder);

   
   //botao associado a imagem 'folder'
   fs_bt = elm_fileselector_button_add(vbox);//botao especifico para selecionar arquivos
   elm_fileselector_button_path_set(fs_bt, "/home");//seta o diretorio inicial ao botao
   elm_object_text_set(fs_bt, "Select a file");
   elm_object_part_content_set(fs_bt, "icon", folder);//adiciona o icone de 'folder' 'a esquerda do nome do botao 'Select a file'
   elm_box_pack_end(vbox, fs_bt);
  //evas_object_size_hint_weight_set(fs_bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_min_set(fs_bt, 100, 30);
 //evas_object_size_hint_align_set(fs_bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(fs_bt);
   
   
/* //adicionar uma funcao para o b_exiting
   en = elm_entry_add(win);//o objeto passa ser uma entrada de texto |ou saida? |elm_entry.h
   elm_entry_line_wrap_set(en, EINA_FALSE);
   elm_entry_editable_set(en, EINA_FALSE);
   evas_object_smart_callback_add(_list, "file,chosen", _file_chosen, en);
   elm_box_pack_end(box, en);
   evas_object_show(en);
*/
   //mostrar um arquivo texto com lista de musicas n = elm_entry_add(win);
 
   label = elm_label_add(win);
   elm_object_text_set(label,opp);
   evas_object_show(label);

   scroll = elm_scroller_add(win);
   evas_object_size_hint_weight_set(scroll, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, scroll);
   evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(scroll);
   
   elm_object_content_set(scroll, label);
   elm_scroller_bounce_set(scroll, EINA_TRUE, EINA_FALSE);
   elm_scroller_policy_set(scroll, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_scroller_propagate_events_set(scroll, EINA_TRUE);
   elm_scroller_page_relative_set(scroll, 0, 1);
   elm_scroller_region_show(scroll, 50, 50, 200, 200);
   //elm_box_pack_end(box, scroll);
  
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);
        
   _list = elm_button_add(box);
   elm_object_text_set(_list, "> Playing...");
  // evas_object_smart_callback_add(_list, "file,chosen", on_done, en);
   evas_object_resize(_list, 150, 30);
   //evas_object_move(_list, 20, 60);
   //elm_box_pack_start(box, _list);
   evas_object_show(_list);
  elm_box_pack_end(vbox, _list);
   elm_run();
   elm_shutdown();
  
 return 0;
}
 ELM_MAIN()

