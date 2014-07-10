<?php
/**
 * @file
 * piwik_autologin.admin.inc
 * @brief Implementing the form dedicated to the module configuration.
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

if (!function_exists('\piwik_autologin\get_piwik_idsite')) {
  require_once 'piwik_autologin.utils.inc';
}

/**
 * Filling the form for module configuration.
 *
 * @param mixed $form
 *   Form to fill.
 * @param mixed $form_state
 *   Form state.
 *
 * @return mixed
 *   The form.
 */
function piwik_autologin_admin_form($form, &$form_state) {
  $form['piwik_url'] = array(
    '#type' => 'textfield',
    '#title' => 'Piwik URL',
    '#description' => 'URL of the Piwik installation which tracks the website.',
    '#default_value' => \piwik_autologin\get_piwik_url(),
    '#size' => 50,
    '#required' => TRUE,
  );

  $form['idsite'] = array(
    '#type' => 'textfield',
    '#title' => 'Website ID',
    '#description' => 'Website ID of the site on the Piwik installation which tracks the website.',
    '#default_value' => \piwik_autologin\get_piwik_idsite(),
    '#size' => 50,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Update',
    '#submit' => array('piwik_autologin_admin_submit'),
  );

  return $form;
}

/**
 * Submit callback for configuration.
 *
 * @param mixed $form
 *   The form
 * @param mixed $form_state
 *   Form informations, with the 'values' field containing the submitted values.
 */
function piwik_autologin_admin_submit($form, &$form_state) {
  $error_messages = array();

  // Store Piwik's URL.
  if (!isset($form_state['values']['piwik_url'])) {
    $error_messages[] = 'Cannot retrieve the Piwik URL';
  }
  elseif (filter_var($form_state['values']['piwik_url'], FILTER_VALIDATE_URL)) {
    \piwik_autologin\set_piwik_url($form_state['values']['piwik_url']);
  }
  else {
    $error_messages[] = sprintf("%s is not a valid URL.", $form_state['values']['piwik_url']);
  }

  // Store website ID on Piwik.
  if (isset($form_state['values']['idsite']) AND is_numeric($form_state['values']['idsite'])) {
    \piwik_autologin\set_piwik_idsite($form_state['values']['idsite']);
  }
  else {
    $error_messages[] = 'Website ID on Piwik has to be an integer value.';
  }

  if (count($error_messages) > 0) {
    $formatted_messages = '';
    foreach ($error_messages as $err_msg) {
      $formatted_messages .= "<li>$err_msg</li>";
    }

    drupal_set_message("<ul>$formatted_messages</ul>", 'error');
  }
  else {
    drupal_set_message('Piwik Autologin configuration was updated.', 'status');
  }
}
