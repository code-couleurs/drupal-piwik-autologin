<?php
/**
 * @file
 * piwik_autologin.backend.inc
 * @brief Module backend, with functions called by hook implementations.
 * @author Romain Ducher <r.ducher@agence-codecouleurs.fr>
 * @license https://www.gnu.org/licenses/gpl-3.0.txt
 * @section LICENSE
 *
 * Copyright 2014 Code Couleurs
 *
 * This file is part of Piwik Autologin.
 *
 * Piwik Autologin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Piwik Autologin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS OR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Piwik Autologin. If not, see <http://www.gnu.org/licenses/>.
 */

namespace piwik_autologin;

if (!function_exists('\piwik_autologin\get_piwik_idsite')) {
  require_once 'piwik_autologin.utils.inc';
}

/**
 * Configuration menu for the module.
 *
 * @param array $items
 *   Menu items
 */
function set_module_configuration_menu(&$items) {
  // Configuration menu.
  $items['admin/config/piwik_autologin'] = array(
    'title' => 'Piwik Autologin Configuration',
    'description' => 'Configuring Piwik Autologin',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('piwik_autologin_admin_form'),
    'access arguments' => array('administer site configuration'),
    'file' => 'piwik_autologin.admin.inc',
    'type' => MENU_LOCAL_TASK,
  );
}

/**
 * Link to Piwik in the Navigation menu.
 *
 * @param array $items
 *   Menu items
 */
function set_piwik_navigation_link(&$items) {
  // Ensure that the user is allowed to show Piwik statistics.
  if (!get_show_piwik_navigation_link()) {
    return;
  }

  // Piwik navigation menu.
  $items['navigation/piwik'] = array(
    'title' => 'Piwik',
    'description' => 'Show the website statistics on Piwik',
    'page callback' => '\piwik_autologin\go_to_piwik',
    'access callback' => TRUE,
    'menu_name' => 'navigation',
    'type' => MENU_NORMAL_ITEM,
  );
}

/**
 * Setting a session variable for the logme URL.
 *
 * @param int $uid
 *   Drupal user ID.
 */
function set_logme_url($uid) {
  $piwik_user = PiwikUser::getByDrupalUserID($uid);
  $logme = ($piwik_user AND !$piwik_user->isEmpty());
  set_show_piwik_navigation_link($logme);
  if ($logme) {
    // Saving logme url.
    $_SESSION['piwik_autologin_logme_url'] = $piwik_user->getLogmeURL();
  }
}

/**
 * Embedding an invisible iframe for automatic login to Piwik in a Drupal page.
 *
 * @param array $page
 *   Drupal page.
 */
function embed_piwik_iframe(&$page) {
  global $theme, $user;

  // Checking if there is a connected user.
  $piwik_user = PiwikUser::getByDrupalUserID($user->uid);
  if (is_null($piwik_user)) {
    return;
  }

  // Retrieving the logme URL.
  if (!isset($_SESSION['piwik_autologin_logme_url'])) {
    /*
     * Not a fresh sign in. The automatic connexion to Piwik has already been established.
     *
     * No need to go further for two reasons:
     * 1°) A logme URL would force a reconnection to Piwik each time the user (re)loads a page. Quite noisy, isn't it?
     * 2°) Security concerns: no need to give the Piwik URL to curious users.
     */
    return;
  }

  // A new user has just signed in to Drupal. Let's connect him to Piwik (with the logme feature).
  $logme_url = $_SESSION['piwik_autologin_logme_url'];
  unset($_SESSION['piwik_autologin_logme_url']);

  // The theme system might not yet be initialized. We need $theme.
  drupal_theme_initialize();

  // Picking a random region to insert the invisible iframe into it.
  $regions = array_keys(system_region_list($theme, REGIONS_VISIBLE));
  do {
    $region = array_pop($regions);
  } while (!isset($page[$region]) AND !empty($regions));

  // If no region, we create one.
  if (empty($regions) AND !isset($page[$region])) {
    $region = 'piwik_autologin';
    $page[$region] = array();
  }

  // Creating the hidden iframe with the logme URL.
  $iframe = sprintf("<iframe src=\"%s\" style=\"display: none;\"></iframe>", $logme_url);

  // Creating the block with the iframe and put it in the selected region of the page.
  $page[$region]['piwik_autologin_block'] = array('#markup' => $iframe);
}

/**
 * Creating databases for the module.
 */
function create_piwik_autologin_db_schema() {
  $table_name = 'piwik_autologin_piwik_users';

  $schema[$table_name] = array(
    'description' => 'Piwik equivalents of Drupal users',
    'fields' => array(
      'id' => array(
        'description' => 'Primary key',
        'type' => 'serial',
        'not null' => TRUE,
        'unsigned' => TRUE,
      ),
      'userlogin' => array(
        'description' => 'User\'s login on Piwik',
        'type' => 'varchar',
        'length' => 100,
        'not null' => TRUE,
      ),
      'md5password' => array(
        'description' => 'MD5 User\'s password on Piwik',
        'type' => 'char',
        'length' => 32,
        'not null' => TRUE,
      ),
      'uid' => array(
        'description' => 'Drupal user ID. Foreign key which references {users}.uid',
        'type' => 'int',
        'size' => 'normal',
        'length' => 10,
        'not null' => TRUE,
        'unsigned' => TRUE,
      ),
    ),
    'primary key' => array('id'),
    'foreign key' => array(
      'drupal_uid' => array(
        'table' => 'users',
        'columns' => array('uid' => 'uid'),
      )
    ),
    'unique key' => array(
      'unique_piwik_user' => array('userlogin'),
      'unique_drupal_user' => array('uid'),
    ),
  );

  return $schema;
}

/**
 * Uninstalling Piwik Autologin.
 */
function uninstall_piwik_autologin() {
  // Delete module configuration.
  variable_del('piwik_autologin_piwik_url');
  variable_del('piwik_autologin_idsite');
}
