/******************************************************************************************************************************/
/* ABLS-AGENT-UPS/src/ups.c                Gestion des onduleurs via le protocole NUT                                         */
/* Projet Abls-Habitat                   Gestion d'habitat                                                03.09.2026 09:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ups.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * ABLS-AGENT-UPS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-UPS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-UPS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

 #include "ups.h"

 struct ABLS_AGENT *Agent = NULL;                                                                     /* Structure de l'agent */
 struct ABLS_UPS_VARS *Agent_vars = NULL;                                               /* Structure des variables de l'agent */

/******************************************************************************************************************************/
/* Ups_create_mnemos: Cree les mnemoniques de l'agent                                                                         */
/* Entrée: l'agent                                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_create_mnemos ( void )
  { Agent_vars->Outlet_1_status  = Mnemo_create_DI ( Agent, "OUTLET_1_STATUS",  "Statut de la prise n°1" );
    Agent_vars->Outlet_2_status  = Mnemo_create_DI ( Agent, "OUTLET_2_STATUS",  "Statut de la prise n°2" );
    Agent_vars->Ups_online       = Mnemo_create_DI ( Agent, "UPS_ONLINE",       "UPS Online" );
    Agent_vars->Ups_charging     = Mnemo_create_DI ( Agent, "UPS_CHARGING",     "UPS en charge" );
    Agent_vars->Ups_on_batt      = Mnemo_create_DI ( Agent, "UPS_ON_BATT",      "UPS sur batterie" );
    Agent_vars->Ups_replace_batt = Mnemo_create_DI ( Agent, "UPS_REPLACE_BATT", "Batteries UPS à changer" );
    Agent_vars->Ups_alarm        = Mnemo_create_DI ( Agent, "UPS_ALARM",        "UPS en alarme !" );

    Agent_vars->Load             = Mnemo_create_AI ( Agent, "LOAD",            "Charge onduleur", "%", AGENT_ARCHIVE_1_MIN );
    Agent_vars->Realpower        = Mnemo_create_AI ( Agent, "REALPOWER",       "Puissance active", "W", AGENT_ARCHIVE_1_MIN );
    Agent_vars->Battery_charge   = Mnemo_create_AI ( Agent, "BATTERY_CHARGE",  "Charge batterie", "%", AGENT_ARCHIVE_1_MIN );
    Agent_vars->Input_voltage    = Mnemo_create_AI ( Agent, "INPUT_VOLTAGE",   "Tension d'entrée", "V", AGENT_ARCHIVE_5_MIN );
    Agent_vars->Battery_runtime  = Mnemo_create_AI ( Agent, "BATTERY_RUNTIME", "Durée de batterie restante", "s", AGENT_ARCHIVE_1_MIN );
    Agent_vars->Battery_voltage  = Mnemo_create_AI ( Agent, "BATTERY_VOLTAGE", "Tension batterie", "V", AGENT_ARCHIVE_1_MIN );
    Agent_vars->Input_hz         = Mnemo_create_AI ( Agent, "INPUT_HZ",        "Fréquence d'entrée", "Hz", AGENT_ARCHIVE_5_MIN );
    Agent_vars->Output_current   = Mnemo_create_AI ( Agent, "OUTPUT_CURRENT",  "Courant de sortie", "A", AGENT_ARCHIVE_1_MIN );
    Agent_vars->Output_hz        = Mnemo_create_AI ( Agent, "OUTPUT_HZ",       "Fréquence de sortie", "Hz", AGENT_ARCHIVE_5_MIN );
    Agent_vars->Output_voltage   = Mnemo_create_AI ( Agent, "OUTPUT_VOLTAGE",  "Tension de sortie", "V", AGENT_ARCHIVE_5_MIN );

    Mnemo_create_DO ( Agent, "LOAD_OFF",        "Coupe la sortie ondulée", TRUE );
    Mnemo_create_DO ( Agent, "LOAD_ON",         "Active la sortie ondulée", TRUE );
    Mnemo_create_DO ( Agent, "OUTLET_1_OFF",    "Désactive la prise n°1", TRUE );
    Mnemo_create_DO ( Agent, "OUTLET_1_ON",     "Active la prise n°1", TRUE );
    Mnemo_create_DO ( Agent, "OUTLET_2_OFF",    "Désactive la prise n°2", TRUE );
    Mnemo_create_DO ( Agent, "OUTLET_2_ON",     "Active la prise n°2", TRUE );
    Mnemo_create_DO ( Agent, "START_DEEP_BAT",  "Active un test de décharge profond", TRUE );
    Mnemo_create_DO ( Agent, "START_QUICK_BAT", "Active un test de décharge léger", TRUE );
    Mnemo_create_DO ( Agent, "STOP_TEST_BAT",   "Stop le test de décharge batterie", TRUE );
  }
/******************************************************************************************************************************/
/* Ups_disconnect: Deconnexion du serveur upsd                                                                                */
/* Entrée: l'agent                                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_disconnect ( void )
  { if (Agent_vars->started)
     { upscli_disconnect( &Agent_vars->upsconn );
       Agent_vars->started = FALSE;
     }

    Mqtt_Send_AI ( Agent, Agent_vars->Load,            0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Realpower,       0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Battery_charge,  0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Input_voltage,   0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Battery_runtime, 0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Battery_voltage, 0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Input_hz,        0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Output_current,  0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Output_hz,       0.0, FALSE );
    Mqtt_Send_AI ( Agent, Agent_vars->Output_voltage,  0.0, FALSE );

    Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "Disconnected from upsd" );
    Agent_send_comm_to_master ( Agent, FALSE );
  }
/******************************************************************************************************************************/
/* Ups_query_one: Envoi une ligne au serveur upsd et récupère sa réponse dans le même buffer                                  */
/* Entrée: l'agent, le buffer et sa taille                                                                                    */
/* Sortie: TRUE si l'échange a abouti                                                                                         */
/******************************************************************************************************************************/
 static gboolean Ups_query_one ( gchar *buffer, gsize size )
  { if ( upscli_sendline( &Agent_vars->upsconn, buffer, strlen(buffer) ) == -1 ) return(FALSE);
    if ( upscli_readline( &Agent_vars->upsconn, buffer, size ) == -1 ) return(FALSE);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Ups_connect: Tentative de connexion au serveur upsd                                                                        */
/* Entrée: l'agent                                                                                                            */
/* Sortie: TRUE si la connexion a abouti                                                                                      */
/******************************************************************************************************************************/
 static gboolean Ups_connect ( void )
  { gchar buffer[128];

    gchar *host           = Agent_config_get_string ( Agent, "host" );
    gchar *name           = Agent_config_get_string ( Agent, "name" );
    gchar *admin_username = Agent_config_get_string ( Agent, "admin_username" );
    gchar *admin_password = Agent_config_get_string ( Agent, "admin_password" );

    if ( upscli_connect( &Agent_vars->upsconn, host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL ) == -1 )
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING,
             "Connexion refused by upsd (host='%s' -> %s)", host, (char *)upscli_strerror(&Agent_vars->upsconn) );
       return(FALSE);
     }

    Agent_vars->started = TRUE;

    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", name );
    if (!Ups_query_one ( buffer, sizeof(buffer) ))
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING,
             "GET UPSDESC failed (%s)", (char *)upscli_strerror(&Agent_vars->upsconn) );
     }
    else
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_DEBUG, "GET UPSDESC -> %s", buffer ); }

    if (admin_username && *admin_username)
     { g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", admin_username );
       if (!Ups_query_one ( buffer, sizeof(buffer) ))
        { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING,
                "USERNAME failed (%s)", (char *)upscli_strerror(&Agent_vars->upsconn) );
        }
     }

    if (admin_password && *admin_password)
     {  g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", admin_password );
       if (!Ups_query_one ( buffer, sizeof(buffer) ))
        { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING,
                "PASSWORD failed (%s)", (char *)upscli_strerror(&Agent_vars->upsconn) );
        }
     }

    Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "Connected (host='%s', name='%s')", host, name );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Ups_set_instcmd: Envoi d'une commande instantanée à l'onduleur                                                             */
/* Entrée: l'agent, le nom de la commande                                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_set_instcmd ( gchar *nom_cmd )
  { gchar buffer[128];

    if (!Agent_vars->started) return;

    gchar *name = Agent_config_get_string ( Agent, "name" );

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", name, nom_cmd );
    if (!Ups_query_one ( buffer, sizeof(buffer) ))
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING,
             "INSTCMD '%s' failed (%s)", nom_cmd, (char *)upscli_strerror(&Agent_vars->upsconn) );
       Ups_disconnect();
       return;
     }
    Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "INSTCMD '%s' -> %s", nom_cmd, buffer );
  }
/******************************************************************************************************************************/
/* Ups_get_var: Recupere la valeur d'une variable NUT                                                                         */
/* Entrée: l'agent, le nom de la variable                                                                                     */
/* Sortie: la valeur sans ses guillemets, NULL si erreur                                                                      */
/******************************************************************************************************************************/
 static gchar *Ups_get_var ( gchar *nom_var )
  { static gchar buffer[128];

    if (!Agent_vars->started) return(NULL);

    gchar *name = Agent_config_get_string ( Agent, "name" );

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", name, nom_var );
    if (!Ups_query_one ( buffer, sizeof(buffer) ))
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING,
             "GET VAR '%s' failed (%s)", nom_var, (char *)upscli_strerror(&Agent_vars->upsconn) );
       Ups_disconnect ();
       return(NULL);
     }

    if ( !strncmp ( buffer, "ERR", 3 ) )                                             /* Detection des erreurs type DATA-STALE */
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_DEBUG, "GET VAR '%s' -> %s", nom_var, buffer );
       return(NULL);
     }

    if ( strncmp ( buffer, "VAR", 3 ) ) return(NULL);        /* VAR NOT SUPPORTED / DRIVER NOT CONNECTED ne sont pas fatals */

    gchar *debut = strchr ( buffer, '"' );                    /* La valeur est le dernier champ, encadré par des guillemets */
    if (!debut) return(NULL);
    debut++;

    gchar *fin = strrchr ( debut, '"' );
    if (fin) *fin = '\0';

    Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_DEBUG, "GET VAR '%s' -> '%s'", nom_var, debut );
    return(debut);
  }
/******************************************************************************************************************************/
/* Ups_interroger: Interrogation de l'onduleur                                                                                */
/* Entrée: l'agent                                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_interroger ( void )
  { gchar *reponse;

    if ( (reponse = Ups_get_var ( "ups.load" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Load, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "ups.realpower" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Realpower, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "battery.charge" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Battery_charge, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "input.voltage" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Input_voltage, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "battery.runtime" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Battery_runtime, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "battery.voltage" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Battery_voltage, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "input.frequency" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Input_hz, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "output.current" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Output_current, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "output.frequency" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Output_hz, atof(reponse), TRUE ); }

    if ( (reponse = Ups_get_var ( "output.voltage" )) != NULL )
     { Mqtt_Send_AI ( Agent, Agent_vars->Output_voltage, atof(reponse), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Ups_get_var ( "outlet.1.status" )) != NULL )
     { Mqtt_Send_DI ( Agent, Agent_vars->Outlet_1_status, !strcasecmp(reponse, "on") ); }

    if ( (reponse = Ups_get_var ( "outlet.2.status" )) != NULL )
     { Mqtt_Send_DI ( Agent, Agent_vars->Outlet_2_status, !strcasecmp(reponse, "on") ); }

    if ( (reponse = Ups_get_var ( "ups.status" )) != NULL )
     { Mqtt_Send_DI ( Agent, Agent_vars->Ups_online,       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       Mqtt_Send_DI ( Agent, Agent_vars->Ups_charging,     (g_strrstr(reponse, "DISCHRG")?FALSE:TRUE) );
       Mqtt_Send_DI ( Agent, Agent_vars->Ups_on_batt,      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       Mqtt_Send_DI ( Agent, Agent_vars->Ups_replace_batt, (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       Mqtt_Send_DI ( Agent, Agent_vars->Ups_alarm,        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
       Agent_send_comm_to_master ( Agent, TRUE );
     }
    else Agent_send_comm_to_master ( Agent, FALSE );
  }
/******************************************************************************************************************************/
/* Ups_SET_DO: Traite une demande de commande TOR en provenance du master                                                     */
/* Entrée: l'agent et le message                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_SET_DO ( JsonNode *msg )
  { gchar *agent_acronyme = Json_get_string ( msg, "mqtt_topic_lvl2" );
    gchar *tech_id        = Json_get_string ( msg, "tech_id" );
    gchar *acronyme       = Json_get_string ( msg, "acronyme" );

    if (!agent_acronyme)
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "Requete mal formée manque mqtt_topic_lvl2" );
       return;
     }

    if (!Json_has_member ( msg, "etat" ))
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "Requete mal formée manque etat" );
       return;
     }

    gboolean etat = Json_get_bool ( msg, "etat" );
    Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "SET_DO '%s:%s'/'%s:%s'=%d",
          Agent->agent_tech_id, agent_acronyme, tech_id, acronyme, etat );

    if (!etat) return;                                                    /* Les commandes onduleur sont des monostables */

         if (!strcasecmp(agent_acronyme, "LOAD_OFF"))        Ups_set_instcmd ( "load.off" );
    else if (!strcasecmp(agent_acronyme, "LOAD_ON"))         Ups_set_instcmd ( "load.on" );
    else if (!strcasecmp(agent_acronyme, "OUTLET_1_OFF"))    Ups_set_instcmd ( "outlet.1.load.off" );
    else if (!strcasecmp(agent_acronyme, "OUTLET_1_ON"))     Ups_set_instcmd ( "outlet.1.load.on" );
    else if (!strcasecmp(agent_acronyme, "OUTLET_2_OFF"))    Ups_set_instcmd ( "outlet.2.load.off" );
    else if (!strcasecmp(agent_acronyme, "OUTLET_2_ON"))     Ups_set_instcmd ( "outlet.2.load.on" );
    else if (!strcasecmp(agent_acronyme, "START_DEEP_BAT"))  Ups_set_instcmd ( "test.battery.start.deep" );
    else if (!strcasecmp(agent_acronyme, "START_QUICK_BAT")) Ups_set_instcmd ( "test.battery.start.quick" );
    else if (!strcasecmp(agent_acronyme, "STOP_TEST_BAT"))   Ups_set_instcmd ( "test.battery.stop" );
    else Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_WARNING, "SET_DO '%s' inconnu", agent_acronyme );
  }
/******************************************************************************************************************************/
/* main: Prend en charge l'agent onduleur                                                                                     */
/* Entrée: les paramètres de la ligne de commande                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gint main ( gint argc, gchar *argv[] )
  { Config_add_parameter ( "host",           "HOST", "Adresse du serveur NUT",          CONFIG_STRING );
    Config_add_parameter ( "name",           "NAME", "Nom de l'onduleur dans NUT",      CONFIG_STRING );
    Config_add_parameter ( "admin-username", "USER", "Utilisateur d'administration NUT", CONFIG_STRING );
    Config_add_parameter ( "admin-password", "PASS", "Mot de passe d'administration NUT", CONFIG_STRING );
    Agent = Agent_init ( argv[0], "ups", ABLS_AGENT_UPS_VERSION, sizeof(struct ABLS_UPS_VARS), argc, argv );
    Agent_vars = Agent->vars;

    if (!Agent_config_get_string ( Agent, "host" ))
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "ERROR: No host, stopping agent" );
       Agent_end(Agent);
     }

    if (!Agent_config_get_string ( Agent, "name" ))
     { Info( __func__, Agent->agent_classe, Agent->agent_tech_id, LOG_ERR, "ERROR: No name, stopping agent" );
       Agent_end(Agent);
     }

    Ups_create_mnemos ( );

    Agent_is_ready ( Agent );

    while(Agent->Agent_run == AGENT_IS_RUNNING)                                              /* On tourne tant que necessaire */
     { Agent_loop ( Agent );                                             /* Loop sur l'agent pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message ( Agent ) ) != NULL )
        { if (Mqtt_topic_is ( mqtt_local_message, 2, "SET_DO", Agent->agent_tech_id ))
           { Ups_SET_DO ( mqtt_local_message ); }
          Json_unref ( mqtt_local_message );
        }
/****************************************************** Ecoute de l'api *******************************************************/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( Agent ) ) != NULL )
        { if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "AGENT", Agent->agent_tech_id, "TEST" ) )
           { Info(__func__, Agent->agent_classe, Agent->agent_tech_id, LOG_NOTICE, "Agent Test from API."); }
          Json_unref (mqtt_api_message);
        }
/****************************************************** Interrogation de l'ups ************************************************/
       time_t now = time(NULL);
       if (now >= Agent_vars->next_connexion)
        { if (!Agent_vars->started)
           { if (!Ups_connect())
              { Ups_disconnect();
                Agent_set_status ( Agent, "Serveur NUT injoignable" );
                Agent_vars->next_connexion = now + UPS_RETRY_SEC;
              }
           }
          else
           { Ups_interroger();
             if (Agent_vars->started) Agent_set_status ( Agent, "Onduleur interrogé" );
             Agent_vars->next_connexion = now + UPS_POLLING_SEC;
           }
        }
     }

    Ups_disconnect();
    Agent_end(Agent);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
