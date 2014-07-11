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

if (!class_exists('\piwik_autologin\PiwikUser')) {
  require_once 'piwik_autologin.piwik_user.inc';
}

if (!class_exists('\piwik_autologin\Session')) {
  require_once 'piwik_autologin.session.inc';
}

/**
 * Configuration menu for the module.
 *
 * @param array $items
 *   Menu items
 */
function set_module_configuration_menu(&$items) {
  // Configuration menu.
  $menu_title = t('Piwik Autologin Configuration');
  $menu_desc = t('Configuring Piwik Autologin');
  $items['admin/config/piwik_autologin'] = array(
    'title' => $menu_title,
    'description' => $menu_desc,
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
  if (!Session::getShowNaviLink()) {
    return;
  }

  // Piwik navigation menu.
  $items['navigation/piwik'] = array(
    'title' => t('Piwik'),
    'description' => t('Show the website statistics on Piwik'),
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

  // Show the Piwik navigation link in the menu (or not).
  Session::setShowNaviLink($logme);

  // Saving logme url.
  if ($logme) {
    Session::setLogmeURL($piwik_user->getLogmeURL());
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
  if (!Session::getLogmeURL(NULL)) {
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
  $logme_url = Session::getLogmeURL();
  Session::delLogmeURL();

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
      ),
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

/**
 * Adding fields to a user form in order to administrate Piwik users parameters.
 *
 * @param array $form
 *   The form where fields will be added.
 * @param string $default_userlogin
 *   Userlogin value. It is empty by default.
 */
function add_piwik_user_fields(&$form, $default_userlogin = '') {
  $form['piwik_autologin_userlogin'] = array(
    '#type' => 'textfield',
    '#title' => t('Piwik user login'),
    '#description' => t('User login of a Piwik equivalent on the Piwik installation which tracks the website. Ensure that this user was at least granted the right to show statistics on Piwik.'),
    '#width' => 100,
    '#required' => FALSE,
    '#default_value' => $default_userlogin,
  );

  $form['piwik_autologin_password'] = array(
    '#type' => 'password',
    '#title' => t('Piwik password'),
    '#description' => t("Piwik equivalent's password on Piwik."),
    '#width' => 100,
    '#required' => FALSE,
    '#default_value' => '',
  );
}

/**
 * Adding a Piwik equivalent to the Drupal Database.
 *
 * @param string $userlogin
 *   User's login on Piwik.
 * @param string $clear_password
 *   User's clear password on Piwik.
 * @param int $uid
 *   Drupal user ID
 */
function add_user($userlogin, $clear_password, $uid) {
  $piwik_user = new PiwikUser($userlogin, $clear_password, $uid, 0, TRUE);
  !$piwik_user->isEmpty() AND $piwik_user->insert();
}

/**
 * Edit Piwik user datas while editing a Drupal user.
 *
 * @param array $form
 *   Form which will receive additional fields.
 * @param int $drupal_uid
 *   Drupal user ID.
 */
function edit_piwik_user_fields(&$form, $drupal_uid) {
  $userlogin = PiwikUser::getUserloginByDrupalUID(intval($drupal_uid));
  if (is_null($userlogin)) {
    $userlogin = '';
  }

  add_piwik_user_fields($form, $userlogin);
}

/**
 * Adding a Piwik equivalent to the Drupal Database.
 *
 * @param string $userlogin
 *   User's login on Piwik.
 * @param string $clear_password
 *   User's clear password on Piwik.
 * @param int $uid
 *   Drupal user ID
 */
function edit_user($userlogin, $clear_password, $uid) {
  $piwik_user = PiwikUser::getByDrupalUserID($uid);
  $empty_userlogin = in_array($userlogin, array(NULL, ''));

  if (is_null($piwik_user)) {
    // Perhaps a new user, who knows?
    !$empty_userlogin AND add_user($userlogin, $clear_password, $uid);
  }
  elseif ($empty_userlogin) {
    // No login => no more Piwik equivalent. Delete it.
    $piwik_user->delete();
  }
  else {
    // Classic update.
    $piwik_user->setUserlogin($userlogin);
    !empty($clear_password) AND $piwik_user->setMD5Password($clear_password, TRUE);
    $piwik_user->update();
  }
}

/**
 * Deleting a Piwik equivalent.
 *
 * @param int $uid
 *   Drupal user ID.
 */
function delete_piwik_user($uid) {
  $piwik_user = PiwikUser::getByDrupalUserID($uid);
  !is_null($piwik_user) AND $piwik_user->delete();
}
