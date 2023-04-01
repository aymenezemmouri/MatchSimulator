#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#define MAX_TEAMS 32
#define DURATION 90 // default match duration is 90 minutes, equivalent to 5400sec
int num_teams;
char *team_names[MAX_TEAMS];
int teams_remaining[MAX_TEAMS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Match{
    int team1;
    int team2;
    int score1;
    int score2;
    int tour;
}*Match;


void *play_match(void *ma)
{
    Match match = (Match) ma;
    //printf("match->team1 %d\n",match->team1);
    //printf("match->team2 %d\n",match->team2);
    int duration = DURATION;
    int action;

    //printf("apres le changement\n");
    // Simulate match by randomly generating scores
    printf("DEBUT %s %d - %d %s duration %d\n",team_names[match->team1],match->score1, match->score2,team_names[match->team2],duration);
    while (duration > 0)
    {
        //printf("duration %d\n",duration);
        action = rand() % 100; // simulate random action
        //printf("Action %d\n",action);
        if (action < 98)
        { // 98% chance of no score
            duration--;
        }
        else if (action == 99 )
        { // 1% chance of goal for team 1
            match->score1++;
            duration--;
            printf("    %.20s %d - %d %-20s\n", team_names[match->team1], match->score1, match->score2, team_names[match->team2]);
        }
        else
        { // 1% chance of goal for team 2
            match->score2++;
            duration--;
            printf("    %.20s %d - %d %-20s\n", team_names[match->team1], match->score1, match->score2, team_names[match->team2]);
        }
        sleep(0.005);
    }
    int nTab = 5;
    while(match->score1 == match->score2){ //SI MATCH NUL, SEANCE DE TAB
        int tab;
        while(nTab > 0){
            tab = rand() % 100;
            if(tab > 20){ //80% de marquer
                match->score1++;
                printf("    %.20s (%d) - (%d) %-20s\n", team_names[match->team1], match->score1, match->score2, team_names[match->team2]);
            }
            tab = rand() % 100;
            if(tab < 40){//60% de marquer
                match->score2++;
                printf("    %.20s (%d) - (%d) %-20s\n", team_names[match->team1], match->score1, match->score2, team_names[match->team2]);
            }
            nTab--;
        }
        nTab=1;
    }
    //printf("fin de la partie \n");
    if(match->score1 > match->score2){
        pthread_mutex_lock(&mutex);
        teams_remaining[match->team1]++;
        teams_remaining[match->team2] = -1;
        pthread_mutex_unlock(&mutex);
        printf("FIN %s* %d - %d %s\n",team_names[match->team1], match->score1, match->score2, team_names[match->team2]);
    }
    else{
        pthread_mutex_lock(&mutex);
        teams_remaining[match->team2]++;
        teams_remaining[match->team1] = -1;
        pthread_mutex_unlock(&mutex);
        printf("FIN %s %d - %d %s*\n",team_names[match->team1], match->score1, match->score2, team_names[match->team2]);
    }
    pthread_exit(0);
}

void printMatch(Match match){
    printf("Match %s %d - %d %s\n",team_names[match->team1],match->score1, match->score2,team_names[match->team2]);
}


int main(int argc, char *argv[])
{

    pthread_mutex_init(&mutex,NULL);

    //Creation d'une liste randomise avec les equipes
    //Fin lecture des equipes

    //=> liste avec les equipes, tel que chaque 2 equipes adjecentes joueront un match
    team_names[0] = "Real Madrid";
    team_names[1] = "FC Barcelone";
    team_names[2] = "Manchester United";
    team_names[3] = "Liverpool";
    team_names[4] = "PSG";
    team_names[5] = "FC Bayern";
    team_names[6] = "Manchester City";
    team_names[7] = "Arsenal";

    num_teams = 8;
    for (int i = 0; i < num_teams; i++)
    {
        teams_remaining[i] = 1;
    }

    if (num_teams % 2 != 0 || num_teams > MAX_TEAMS)
    {
        printf("Number of teams must be a power of 2 and less than %d\n", MAX_TEAMS);
        exit(EXIT_FAILURE);
    }

    int num_matchs_tour = num_teams / 2;
    int num_matchs_total = 1;
    int tour = 1;
    for (int i = num_teams / 2; i > 1; i/=2)
    {
        tour++;
        num_matchs_total += i;
    }
    //printf("Num matchs %d et tour %d\n",num_matchs_tour,tour);

    // Create threads to play matches
    pthread_t threads[num_matchs_total];
    Match matchs[num_matchs_total];

    //printf("Nombre de match au total %d\n",num_matchs_total);
    int team2 = 0;
    int num_match = 0;
    int t;

    // for (int i = 0; i < num_teams; i++)
    // {
    //     printf("i=%d et t=%d\n",i,teams_remaining[i]);
    // }
    while(num_match < num_matchs_total){
        //printf("une fois\n");
        for (int team1 = 0; team1 < num_teams; team1++)
        {
            pthread_mutex_lock(&mutex);
            //printf("apres le mutex\n");
            if(teams_remaining[team1] > 0){
                //printf("oui\n");
                t = teams_remaining[team1]; //le tour
                //printf("t=%d\n",t);
                team2 = team1+1;
                //printf("team1 == %d et team2 == %d\n",team1,team2);
                while(team2 < num_teams){
                    if(teams_remaining[team2] == t){
                        //printf("num match %d\n",num_match);
                        matchs[num_match] = (Match)malloc(sizeof(struct Match));
                        //printf("ici\n");
                        matchs[num_match]->team1 = team1;
                        //printf("la bas\n");
                        matchs[num_match]->team2 = team2;
                        //printf("autre part\n");
                        matchs[num_match]->score1 = 0;
                        matchs[num_match]->score2 = 0;
                        matchs[num_match]->tour = teams_remaining[team2];
                        //printf("avant le mutex\n");
                        //pthread_mutex_lock(&mutex);
                        //printf("apres le mutex\n");
                        teams_remaining[team1] = 0;
                        teams_remaining[team2] = 0;
                        //printf("apres la cc\n");
                        //pthread_mutex_unlock(&mutex);
                        //printf("apres le unlock mutex\n");
                        //printf("creation d'un match entre %d et %d \n",team1,team2);
                        //printf("team2 %d\n",team2);
                        pthread_create(&threads[num_match], NULL, play_match, matchs[num_match]);
                        num_match++;
                        team2++;
                        break;
                    }
                    team2++;
                    //printf("team2 = %d\n",team2);
                }
            }
            pthread_mutex_unlock(&mutex);
            //printf("c bon\n");
        }
        //pthread_create(&threads[j], NULL, play_match, matchs[num_match]);
    }

    for (int i = 0; i < num_matchs_total; i++)
    {
        pthread_join(threads[i], NULL);
    }


    pthread_mutex_destroy(&mutex);


    return 1;
}
