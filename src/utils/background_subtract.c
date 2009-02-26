#include <spimage.h>
#include <strings.h>

typedef struct{
  int n_pulses;
  char ** name;
  int * FEL_Timestamp;
  real * Tunnel_Ion;
  real * Hall_Ion;
  /* this one only serves to optimize lookup time */
  int search_index;
}Pulse_Data;

/*int lookup_by_timestamp(Pulse_Data * pd, int timestamp, int window){
  int i;
  for(i = pd->search_index-window;i<pd->search_index+window;i++){
    if(pd->FEL_Timestamp[(i+pd->n_pulses)%pd->n_pulses] == timestamp){
      if(pd->Tunnel_Ion[(i+pd->n_pulses)%pd->n_pulses] < -0.99){
	pd->search_index++;
	return pd->search_index+i-1;
      }
    }
  }
  return -1;
}
*/

void image_normalize(Image * in){
  real sum;
  return;
  for(int i = 0;i<sp_image_size(in);i++){
    sum += sp_real(in->image->data[i]);
  }
  for(int i = 0;i<sp_image_size(in);i++){
    sp_real(in->image->data[i]) /= sum;
  } 
}

int lookup_by_timestamp(Pulse_Data * pd, int timestamp){
  int i;
  for(i = 0;i<pd->n_pulses;i++){
    if(pd->FEL_Timestamp[i] == timestamp){
      if(pd->Tunnel_Ion[(i+pd->n_pulses)%pd->n_pulses] < -0.99){
	return i;
      }
    }
  }
  return -1;
}

int lookup_by_name(Pulse_Data * pd, char * name){
  int i;
  for(i = 0;i<pd->n_pulses;i++){
    if(strcmp(pd->name[pd->search_index],name) == 0){
      return pd->search_index;
    }
    pd->search_index = (pd->search_index + 1)%pd->n_pulses;
  }
  return -1;
}

void interpolate_missing(Pulse_Data * pd){
  for(int i = 0;i<pd->n_pulses;i++){
    if(pd->Tunnel_Ion[i] < -0.9){
      if(i == 0){
	pd->Tunnel_Ion[i] = pd->Tunnel_Ion[i+1];
	pd->Hall_Ion[i] = pd->Hall_Ion[i+1];
      }else if(i == pd->n_pulses -1){
	pd->Tunnel_Ion[i] = pd->Tunnel_Ion[i-1];
	pd->Hall_Ion[i] = pd->Hall_Ion[i-1];
      }else{
	pd->Tunnel_Ion[i] = (pd->Tunnel_Ion[i-1]+
			     pd->Tunnel_Ion[i+1])/2;
	pd->Hall_Ion[i] = (pd->Hall_Ion[i-1]+
			   pd->Hall_Ion[i+1])/2;
      }
    }
  }
}

Pulse_Data * read_pulse_data(char * MasterLog, char * FEL_PulseInfo){
  FILE * ml =fopen(MasterLog,"r");
  FILE * pi = fopen(FEL_PulseInfo,"r");
  char buffer[1024];
  char name[1024];
  int timestamp;
  int i=0;
  int index;
  real ti,hi;
  Pulse_Data * ret = malloc(sizeof(Pulse_Data));
  while(fgets(buffer,1024,ml)){
    if(strstr(buffer,"FEL") == buffer){
      i++;
    }    
  }
  ret->search_index = 0;
  ret->n_pulses = i;
  ret->FEL_Timestamp = malloc(sizeof(int)*i);
  ret->Tunnel_Ion = malloc(sizeof(real)*i);
  ret->Hall_Ion = malloc(sizeof(real)*i);
  ret->name = malloc(sizeof(char *)*i);
  fclose(ml);
  ml =fopen(MasterLog,"r");
  i = 0;
  while(fgets(buffer,1024,ml)){
    if(strstr(buffer,"FEL") == buffer){
      /* Line starts with FEL, we're gonna assume it's a pulse */
      sscanf(buffer,"%s %*d/%*d/%*d, %*d:%*d:%*d, %*s %*s %*s %d",name,&timestamp);      
      /* chop comma off the name*/
      name[strlen(name)-1] = 0;
      ret->FEL_Timestamp[i] = timestamp;
      ret->name[i] = malloc(sizeof(char )*10);
      ret->Tunnel_Ion[i] = -1;
      strcpy(ret->name[i],name);
      i++;

    }
  }
  printf("%d Pulses read\n",i);
  printf("Parsing %s...\n",FEL_PulseInfo);
  pi =fopen(FEL_PulseInfo,"r");
  while(fgets(buffer,1024,pi)){
    sscanf(buffer,"%d, %*d/%*d/%*d, %*d:%*d:%*d, %*d, %*d, %*f, %f, %*f, %f",&timestamp,&ti,&hi);      
    index = lookup_by_timestamp(ret,timestamp);
    if(index != -1){
      ret->Tunnel_Ion[index] = ti;
      ret->Hall_Ion[index] = hi;
      i--;
    }

  }
  printf("%d pulses not found in CSV\n",i);
  printf("Interpolating missing values\n");
  interpolate_missing(ret);
  return ret;
}

real image_correlation(Image * a, Image * b, Image * corr){
  real sum_sq_x = 0;
  real sum_sq_y = 0;
  real sum_coproduct = 0;
  real mean_x = sp_real(a->image->data[0]);
  real sweep;
  real delta_x;
  real delta_y;
  real mean_y = sp_real(b->image->data[1]);
  real pop_sd_x;
  real pop_sd_y;
  real cov_x_y;
  real correlation;
  int k;
  int i;
  /* Dummy statement*/
  corr = NULL;
  for( i=2,k=2;i<=sp_image_size(a);i++){
    if(a->mask->data[i] && b->mask->data[i]){
      sweep = (k - 1.0) / k;
      delta_x = sp_real(a->image->data[i-1]) - mean_x;
      delta_y = sp_real(b->image->data[i-1]) - mean_y;
      sum_sq_x += delta_x * delta_x * sweep;
      sum_sq_y += delta_y * delta_y * sweep;
      sum_coproduct += delta_x * delta_y * sweep;
      mean_x += delta_x / k;
      mean_y += delta_y / k ;
      k++;
    }
  }
  pop_sd_x = sqrt( sum_sq_x / k );
  pop_sd_y = sqrt( sum_sq_y / k );
  cov_x_y = sum_coproduct / k;
  correlation = cov_x_y / (pop_sd_x * pop_sd_y);
  return correlation;
}

real scale_image_to_background(Image * img, Image * bg){
  real img_ph = 0;
  real bg_ph = 0;
  real cc;
  real max_cc;
  real max_scale;
  Image * tmp;
  real scale;
  /* set all points below 750 as not background */
  for(int i = 0;i<sp_image_size(bg);i++){
    if(sp_real(bg->image->data[i]) < 750){
      bg->mask->data[i] = 0;
    }else{
      bg->mask->data[i] = 1;
      bg_ph += sp_real(bg->image->data[i]);
      img_ph += sp_real(img->image->data[i]);
    }
  }
  tmp = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  cc = image_correlation(tmp,bg,0);
  printf("Initial cc - %f\n",cc);
  max_cc = cc;
  max_scale = 1.0;
  sp_image_scale(tmp,bg_ph/img_ph);
  cc = image_correlation(tmp,bg,0);
  if(cc > max_cc){
    max_cc = cc;
    max_scale = bg_ph/img_ph;
  }
  printf("After rough scaling - %f\n",cc);
  scale = 0.1*bg_ph/img_ph;
  sp_image_scale(tmp,0.1);
  for(int i = 0;i<20;i++){
    cc = image_correlation(tmp,bg,0);
    if(cc > max_cc){
      max_cc = cc;
      max_scale = scale;
    }
    
    sp_image_scale(tmp,1.1);
    scale *= 1.1;    
  }  
  sp_image_free(tmp);
  printf("After maximization - %f\n",cc);
  sp_image_scale(img,max_scale);
  for(int i = 0;i<sp_image_size(bg);i++){
    bg->mask->data[i] = 1;
  }
  
  return 0;
}

real calculate_avg_background_correlation(char ** bg_list,int bg_list_n){
  Image * list[5];  
  real total_corr = 0;
  int n = 0;
  for(int i = 0;i<sp_min(bg_list_n,5);i++){
    list[i] = sp_image_read(bg_list[i],0);
  }
  for(int i = 0;i<sp_min(bg_list_n,5);i++){
    for(int j = i+1;j<sp_min(bg_list_n,5);j++){
      total_corr += image_correlation(list[i],list[j],NULL);
      n++;
    }
  }
  for(int i = 0;i<sp_min(bg_list_n,5);i++){
    sp_image_free(list[i]);
  }
  return total_corr/n;
}

char ** read_filelist(char * filename, int  * n){
  char buffer[1024];
  FILE * f = fopen(filename,"r");
  *n = 0;
  while(fgets(buffer,1024,f)){
    (*n)++;
  }
  fclose(f);
  char ** res = malloc(sizeof(char *)*(*n));
  f = fopen(filename,"r");
  for(int i = 0; i < *n;i++){
    fscanf(f,"%s",buffer);
    res[i] = malloc((strlen(buffer)+1)*sizeof(char));
    strcpy(res[i],buffer);
  }
  return res;
}

int main(int argc, char ** argv){
  Image * back;
  Image * tmp;
  char buffer[1024];
  Pulse_Data * pd;
  FILE * hit_list;
  int n_hits = 0;
  int hits_in_a_row = 0;
  int i;
  hit_list = fopen("hit_list.txt","w");
  if(argc < 3){
    printf("background_subtract <background filelist> <image filelist> [MasterLog.txt FEL_PulseInfo.csv]\n");
    exit(0);
  }
  if(argc < 5){
    printf("Pulse Info file not specified! Won't be able to scale images appropriately :-(\n");
  }
  if(argc == 5){
    pd = read_pulse_data(argv[3],argv[4]);    
  }
  int bg_list_n;
  char ** bg_list = read_filelist(argv[1],&bg_list_n);
  int img_list_n;
  char ** img_list = read_filelist(argv[2],&img_list_n);
  back = sp_image_read(bg_list[0],0);
  image_normalize(back);
  real avg_corr = calculate_avg_background_correlation(bg_list,bg_list_n);
  printf("Avg corr - %f\n",avg_corr);

  for(i = 1;i<bg_list_n;i++){
    tmp = sp_image_read(bg_list[i],0);
    image_normalize(tmp);
    sp_image_add(back,tmp);    
    sp_image_free(tmp);
  }
  sp_image_scale(back,1.0/bg_list_n);


  int window = 3;

  for(i = 0;i<img_list_n;i++){
    if(hits_in_a_row > 10){
      printf("Too many hits in a row!. You're advised to move the rest of the files to a new directory and restart again\n");
      printf("Current image - %s\n",img_list[i]);
      printf("I'm gonna try to restart the background by using the previous 10 images\n");
      sp_image_free(back);
      back = sp_image_read(img_list[i-3],0);
      for(int j = -2;j <0;j++){
	tmp = sp_image_read(img_list[i+j],0);
	sp_image_add(back,tmp);
	sp_image_free(tmp);
      }
      sp_image_scale(back,1.0/3);
      hits_in_a_row = 0;
    }
    tmp = sp_image_read(img_list[i],0);
    image_normalize(tmp);
    real corr = image_correlation(tmp, back, NULL);
    printf("Image - %s Corr BG - %f\n",img_list[i],corr);
    strcpy(buffer,img_list[i]);
    /* get basename */
    *(index(buffer,'.')) = 0;
/*    int k = lookup_by_name(pd,buffer);
    printf("Hall Ion - %f\n",pd->Hall_Ion[k]);*/
    if(corr < 0.95){
      n_hits++;
      fprintf(hit_list,"%s\n",img_list[i]);
      printf("!A Hit!\n");
      strcpy(buffer,"bg_sub_");
      strcat(buffer,img_list[i]);    
/*      scale_image_to_background(tmp, back);*/
      sp_image_sub(tmp,back);    
      sp_image_write(tmp,buffer,COLOR_JET|LOG_SCALE);
      hits_in_a_row++;
    }else{
      sp_image_scale(back,window-1);
      sp_image_add(back,tmp);
      sp_image_scale(back,1.0/window);      
      hits_in_a_row = 0;
    }
    sp_image_free(tmp);

  }  
  fclose(hit_list);
}
