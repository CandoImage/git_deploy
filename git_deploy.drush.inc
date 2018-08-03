<?php

/**
 * Implements hook_drush_command().
 */
function git_deploy_drush_command() {
  $items = array();
  $items['git-deploy-fetch'] = array(
    'description' => 'Forces a fresh fetch of all the module / theme informations.',
    'callback' => 'drush_git_deploy_fetch',
    'examples' => array(
      'drush git-deploy-fetch' => 'Fetch all info of all modules / themes.',
      'drush git-deploy-fetch views' => 'Fetch all info of the views module.',
    ),
    'arguments' => array(
      'module' => 'The name of a module to process',
    ),
  );

  return $items;
}

function drush_git_deploy_fetch() {
  $modules = system_list('module_enabled');
  $themes = system_list('theme');

  // Set the batch operations.
  $batch = array(
    'title' => dt('Processing module / theme'),
    'operations' => array(),
  );

  $dedicated_modules = func_get_args();
  if (empty($dedicated_modules)) {
    foreach ($modules as $module) {
      $batch['operations'][] = array('drush_git_deploy_fetch_process_item', array($module));
    }
    foreach ($themes as $theme) {
      $batch['operations'][] = array('drush_git_deploy_fetch_process_item', array($theme));
    }
  }
  else {
    foreach ($dedicated_modules as $module) {
      if (isset($modules[$module])) {
        drush_log(dt('Found module @module - added to queue.', array('@module' => $module)), 'success');
        $batch['operations'][] = array('drush_git_deploy_fetch_process_item', array($modules[$module]));
      }
      if (isset($themes[$module])) {
        drush_log(dt('Found theme @module - added to queue.', array('@module' => $module)), 'success');
        $batch['operations'][] = array('drush_git_deploy_fetch_process_item', array($themes[$module]));
      }
    }
  }
  drush_log(dt('Processing batch with !items items.', array('!items' => count($batch['operations']))), 'success');
  batch_set($batch);
  drush_backend_batch_process();
}

function drush_git_deploy_fetch_process_item($item) {
  drush_log(dt('Processing: @item', array('@item' => $item->name)), 'success');
  $module = git_deploy_update_version_info_worker($item);
  drush_log(dt('        detected version: @version', array('@version' => $module->info['version'])), 'success');
}