/*
    This file is part of the Geany Prettify distribution.

    https://github.com/senselogic/GEANY_PRETTIFY

    Copyright (C) 2017 Eric Pelzer (ecstatic.coder@gmail.com)

    Geany Prettify is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Geany Prettify is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Geany Prettify.  If not, see <http://www.gnu.org/licenses/>.
*/

// -- CONSTANTS

#define GETTEXT_PACKAGE "prettify"
#define LOCALEDIR "locale"
#define VERSION "0.1.1"

// -- IMPORTS

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LOCALE_H
    #include <locale.h>
#endif

#include <geanyplugin.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <errno.h>

// -- VARIABLES

static gchar
    * ConfigurationFilePath = NULL,
    * ConfigurationFolderPath = NULL,
    * PrettifyPath = NULL;
static GtkWidget
    * MenuWidget = NULL;

// -- DECLARATIONS

PLUGIN_VERSION_CHECK( 224 )

// ~~

PLUGIN_SET_TRANSLATABLE_INFO(
    LOCALEDIR,
    GETTEXT_PACKAGE,
    _( "Prettify" ),
    _( "Source code prettifier" ),
    VERSION,
    "SenseLogic <ecstatic.coder@gmail.com>"
    )

// -- VARIABLES

GeanyPlugin
    * geany_plugin;
GeanyData
    * geany_data;

// -- FUNCTIONS

gchar * GetFixedFileText(
    gchar * file_text,
    gint file_text_length,
    gchar * file_path
    )
{
    gchar
        * file_name,
        * file_path_character,
        * fixed_file_path,
        * fixed_file_text,
        * system_command;
    gint
        fixed_file_text_length;
    FILE
        * fixed_file;

    file_name = file_path;

    for ( file_path_character = file_path;
          *file_path_character;
          ++file_path_character )
    {
        if ( *file_path_character == '/' )
        {
            file_name = file_path_character + 1;
        }
    }

    fixed_file_text = 0;

    if ( *file_name )
    {
        fixed_file_path
            = g_strconcat(
                  "/tmp/",
                  file_name,
                  NULL
                  );

        system_command
            = g_strconcat(
                  PrettifyPath,
                  " \"",
                  fixed_file_path,
                  "\"",
                  NULL
                  );

        fixed_file = fopen( fixed_file_path, "wb" );

        if ( fixed_file != 0 )
        {
            fwrite( file_text, 1, file_text_length, fixed_file );
            fclose( fixed_file );

            if ( system( system_command ) >= 0 )
            {
                fixed_file = fopen( fixed_file_path, "rb" );

                if ( fixed_file != 0 )
                {
                    fseek( fixed_file, 0, SEEK_END );
                    fixed_file_text_length = ftell( fixed_file );
                    fixed_file_text = ( gchar * )malloc( fixed_file_text_length + 1 );

                    fseek( fixed_file, 0, SEEK_SET );

                    if ( fread( fixed_file_text, 1, fixed_file_text_length, fixed_file )
                         == fixed_file_text_length )
                    {
                        fixed_file_text[ fixed_file_text_length ] = 0;
                    }
                    else
                    {
                        *fixed_file_text = 0;
                    }

                    fclose( fixed_file );
                }
            }
        }

        g_free( system_command );
        g_free( fixed_file_path );
    }

    return fixed_file_text;
}

// ~~

static void ManageActivation(
    G_GNUC_UNUSED GtkMenuItem * menuitem,
    G_GNUC_UNUSED gpointer gdata
    )
{
    gchar
        * file_path,
        * file_text,
        * fixed_file_text;
    gint
        character_index,
        line_index,
        file_text_length;
    GeanyDocument
        * document;
    ScintillaObject
        * scintilla_object;

    document = document_get_current();

    if ( document != 0 )
    {
        scintilla_object = document->editor->sci;

        sci_start_undo_action( scintilla_object );
        sci_scroll_caret( scintilla_object );

        character_index = sci_get_current_position( scintilla_object );
        line_index = sci_get_line_from_position( scintilla_object, character_index );

        file_path = document->file_name;

        file_text_length = sci_get_length( document->editor->sci );
        file_text = ( gchar * )malloc( file_text_length + 1 );
        file_text = sci_get_contents( scintilla_object, file_text_length + 1 );

        fixed_file_text = GetFixedFileText( file_text, file_text_length, file_path );

        if ( fixed_file_text != 0 )
        {
            if ( *fixed_file_text )
            {
                sci_set_text( scintilla_object, fixed_file_text );
                sci_set_current_position( scintilla_object, character_index, TRUE );
                sci_goto_line( scintilla_object, line_index, TRUE );
            }

            free( fixed_file_text );
        }

        free( file_text );

        sci_end_undo_action( document->editor->sci );
    }
}

// ~~

void WriteConfigurationFile(
    void
    )
{
    GKeyFile
        * key_file;
    gchar
        * configuration_folder_path,
        * data;

    key_file = g_key_file_new();

    g_key_file_set_string( key_file, "prettify", "prettify_path", PrettifyPath );

    data = g_key_file_to_data( key_file, NULL, NULL );
    utils_write_file( ConfigurationFilePath, data );
    g_free( data );

    g_key_file_free( key_file );
}

// ~~

void ReadConfigurationFile(
    void
    )
{
    GKeyFile
        * key_file;

    key_file = g_key_file_new();
    g_key_file_load_from_file( key_file, ConfigurationFilePath, G_KEY_FILE_NONE, NULL );

    g_free( PrettifyPath );
    PrettifyPath = utils_get_setting_string( key_file, "prettify", "prettify_path", "prettify" );

    g_key_file_free( key_file );
}

// ~~

static void ManageConfigurationPanelResponse(
    GtkDialog * dialog,
    gint response,
    gpointer user_data
    )
{
    if ( response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY )
    {
        strcpy(
            PrettifyPath,
            gtk_entry_get_text( GTK_ENTRY( g_object_get_data( G_OBJECT( dialog ), "prettify_path_text_widget" ) ) )
            );

        WriteConfigurationFile();
    }
}

// ~~

void plugin_init(
    G_GNUC_UNUSED GeanyData * data
    )
{
    ConfigurationFolderPath
        = g_strconcat(
              geany->app->configdir,
              G_DIR_SEPARATOR_S,
              "plugins",
              G_DIR_SEPARATOR_S,
              "prettify",
              NULL
              );

    if ( !g_file_test( ConfigurationFolderPath, G_FILE_TEST_IS_DIR ) )
    {
        utils_mkdir( ConfigurationFolderPath, TRUE );
    }

    ConfigurationFilePath
        = g_strconcat(
              ConfigurationFolderPath,
              G_DIR_SEPARATOR_S,
              "prettify.conf",
              NULL
              );

    ReadConfigurationFile();

    MenuWidget = gtk_image_menu_item_new_with_mnemonic( _( "_Prettify" ) );

    gtk_widget_set_tooltip_text( MenuWidget, _( "Source code prettifier" ) );
    gtk_widget_show( MenuWidget );

    g_signal_connect(
        ( gpointer ) MenuWidget,
        "activate",
        G_CALLBACK( ManageActivation ),
        NULL
        );

    gtk_container_add(
        GTK_CONTAINER( geany->main_widgets->tools_menu ),
        MenuWidget
        );
}

// ~~

GtkWidget * plugin_configure(
    GtkDialog * dialog
    )
{
    GtkWidget
        * prettify_path_hbox_widget,
        * prettify_path_label_widget,
        * prettify_path_text_widget,
        * vbox_widget;

    vbox_widget = gtk_vbox_new( FALSE, 6 );

    prettify_path_text_widget = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY( prettify_path_text_widget ), PrettifyPath );
    ui_entry_add_clear_icon( GTK_ENTRY( prettify_path_text_widget ) );
    gtk_widget_set_tooltip_text( prettify_path_text_widget, _( "Prettify path." ) );

    prettify_path_label_widget = gtk_label_new_with_mnemonic( _( "Prettify path :" ) );
    gtk_label_set_mnemonic_widget( GTK_LABEL( prettify_path_label_widget ), prettify_path_text_widget );

    prettify_path_hbox_widget = gtk_hbox_new( FALSE, 0 );
    gtk_box_pack_start( GTK_BOX( prettify_path_hbox_widget ), prettify_path_label_widget, FALSE, FALSE, 3 );
    gtk_box_pack_start( GTK_BOX( prettify_path_hbox_widget ), prettify_path_text_widget, TRUE, TRUE, 3 );

    gtk_container_add( GTK_CONTAINER( vbox_widget ), prettify_path_hbox_widget );

    g_object_set_data( G_OBJECT( dialog ), "prettify_path_text_widget", prettify_path_text_widget );

    g_signal_connect( dialog, "response", G_CALLBACK( ManageConfigurationPanelResponse ), NULL );

    gtk_widget_show_all( vbox_widget );

    return vbox_widget;
}

// ~~

void plugin_cleanup(
    void
    )
{
    gtk_widget_destroy( MenuWidget );

    g_free( ConfigurationFolderPath );
    g_free( ConfigurationFilePath );
    g_free( PrettifyPath );
}
