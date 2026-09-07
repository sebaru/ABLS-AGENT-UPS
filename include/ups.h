/******************************************************************************************************************************/
/* ABLS-AGENT-UPS/include/ups.h            Header de l'agent Onduleur                                                         */
/* Projet Abls-Habitat                   Gestion d'habitat                                                03.09.2026 09:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ups.h
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

 #ifndef _ABLS_UPS_H_
 #define _ABLS_UPS_H_

 #include <upsclient.h>

 #include <abls-agent-libs/abls-agent-libs.h>

 #define UPS_PORT_TCP      3493                                                 /* Port de connexion TCP pour accès au daemon */
 #define UPS_POLLING_SEC   10                                                       /* Interrogation nominale du serveur upsd */
 #define UPS_RETRY_SEC     180                                                 /* Attente avant retry si connexion impossible */

struct ABLS_UPS_VARS
 { UPSCONN_t upsconn;                                                                            /* Connexion au serveur upsd */
   gboolean started;
   time_t next_connexion;
/************************************************************ Analog Input ****************************************************/
   JsonNode *Load;
   JsonNode *Realpower;
   JsonNode *Battery_charge;
   JsonNode *Input_voltage;
   JsonNode *Battery_runtime;
   JsonNode *Battery_voltage;
   JsonNode *Input_hz;
   JsonNode *Output_current;
   JsonNode *Output_hz;
   JsonNode *Output_voltage;
/*********************************************************** Digital Input ****************************************************/
   JsonNode *Outlet_1_status;
   JsonNode *Outlet_2_status;
   JsonNode *Ups_online;
   JsonNode *Ups_charging;
   JsonNode *Ups_on_batt;
   JsonNode *Ups_replace_batt;
   JsonNode *Ups_alarm;
 };

 #endif /* _ABLS_UPS_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
