<?php

/**
 * @file
 * Administration forms for Theme Setup Builder.
 */

/**
 * Builds the Content types export form.
 */
function theme_setup_builder_config_form($form, &$form_state) {
  $options = array();

  foreach (node_type_get_types() as $type => $info) {
    $options[$type] = $info->name;
  }

  return theme_setup_builder_export_form(
    $form,
    'content-types',
    t('Content types'),
    $options
  );
}

/**
 * Builds the Taxonomy export form.
 */
function theme_setup_builder_taxonomy_form($form, &$form_state) {
  $options = array();

  foreach (taxonomy_get_vocabularies() as $vocabulary) {
    $options[$vocabulary->machine_name] = $vocabulary->name;
  }

  return theme_setup_builder_export_form(
    $form,
    'taxonomy',
    t('Taxonomy vocabularies'),
    $options
  );
}

/**
 * Builds the Menus export form.
 */
function theme_setup_builder_menus_form($form, &$form_state) {
  return theme_setup_builder_export_form(
    $form,
    'menus',
    t('Menus'),
    menu_get_menus()
  );
}

/**
 * Builds a shared configuration export form.
 */
function theme_setup_builder_export_form($form, $section, $title, $options) {
  $form['description'] = array(
    '#markup' => '<p>' . t('Select configuration to export for your theme.') . '</p>',
  );

  $form['items'] = array(
    '#type' => 'checkboxes',
    '#title' => $title,
    '#options' => $options,
  );


  $form['items']['#description'] = '<span style="color: #175a91; font-weight: 600;">'
  . t('Select only custom or modified configurations required by your theme. Avoid exporting default configurations unless you have customized them.')
  . '</span>';

  // Store the section on the server-side form definition.
  $form['#export_section'] = $section;
  $form['#export_options'] = $options;

  $form['storage_notice'] = array(
    '#markup' => '<p>' . t('Exported files are stored in the public file system. Avoid exporting sensitive configuration on production sites.') . '</p>',
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export selected'),
    '#submit' => array('theme_setup_builder_config_submit'),
  );

  return $form;
}

/**
 * Handles configuration export for all Config sections.
 */
function theme_setup_builder_config_submit($form, &$form_state) {
  $sections = array(
    'content-types' => 'node.type.',
    'taxonomy' => 'taxonomy.vocabulary.',
    'menus' => 'menu.menu.',
    'image-styles' => 'image.style.',
    'text-formats' => 'filter.format.',
    'custom-blocks' => 'block.custom.',
    'views' => 'views.view.',
    'layouts' => 'layout.layout.',
  );

  $section = $form['#export_section'];

    $handlers = array(
    'fields' => 'theme_setup_builder_export_fields',
    'modules' => 'theme_setup_builder_export_modules',
    'menu-links' => 'theme_setup_builder_export_menu_links',
    'taxonomy-terms' => 'theme_setup_builder_export_taxonomy_terms',
    'content' => 'theme_setup_builder_export_content',
    );

    if (isset($handlers[$section])) {
    $handlers[$section]($form_state);
    return;
    }

  if (!isset($sections[$section])) {
    backdrop_set_message(t('Invalid configuration section.'), 'error');
    return;
  }

  $selected = array_filter($form_state['values']['items']);

  if (empty($selected)) {
    backdrop_set_message(t('Please select at least one item.'), 'warning');
    return;
  }

  $directory = 'public://theme_setup_builder/config';

  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    backdrop_set_message(t('Unable to prepare the export directory.'), 'error');
    return;
  }

  $prefix = $sections[$section];
  $available = $form['#export_options'];
  $exported = 0;

  foreach ($selected as $name) {
    // Only export items listed in the form.
    if (!isset($available[$name])) {
      continue;
    }

    $config_name = $prefix . $name;

    if (theme_setup_builder_export_config($config_name, $directory)) {
      $exported++;

        // Export editor settings associated with the text format.
        if ($section === 'text-formats') {
            $editor_config = 'editor.editor.' . $name;
            theme_setup_builder_export_config($editor_config, $directory);
        }
    }
  }

  backdrop_set_message(t('Exported @count of @total configuration items.', array(
    '@count' => $exported,
    '@total' => count($selected),
  )));
}

/**
 * Exports an active CMI configuration object to JSON.
 */
function theme_setup_builder_export_config($config_name, $directory) {
  $config = config($config_name);
  $data = $config->getOriginal();

  if (empty($data)) {
    return FALSE;
  }

  $json = backdrop_json_encode($data, TRUE);

  if ($json === FALSE) {
    return FALSE;
  }

  $destination = $directory . '/' . $config_name . '.json';

  return file_unmanaged_save_data(
    $json,
    $destination,
    FILE_EXISTS_REPLACE
  ) !== FALSE;
}

/**
 * Writes data as a JSON file in the requested directory.
 */
function theme_setup_builder_write_json($data, $directory, $filename, $messages) {
  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    backdrop_set_message($messages['directory'], 'error');
    return FALSE;
  }

  $json = backdrop_json_encode($data, TRUE);

  if ($json === FALSE) {
    backdrop_set_message($messages['encode'], 'error');
    return FALSE;
  }

  if (file_unmanaged_save_data(
    $json,
    $directory . '/' . $filename,
    FILE_EXISTS_REPLACE
  ) === FALSE) {
    backdrop_set_message($messages['save'], 'error');
    return FALSE;
  }

  return TRUE;
}

/**
 * Displays a Config secondary tab.
 */
function theme_setup_builder_config_tab_page($tab) {
  $forms = array(
    'content-types' => 'theme_setup_builder_config_form',
    'taxonomy' => 'theme_setup_builder_taxonomy_form',
    'menus' => 'theme_setup_builder_menus_form',
    'image-styles' => 'theme_setup_builder_image_styles_form',
    'text-formats' => 'theme_setup_builder_text_formats_form',
    'custom-blocks' => 'theme_setup_builder_custom_blocks_form',
    'views' => 'theme_setup_builder_views_form',
    'layouts' => 'theme_setup_builder_layouts_form',
  );

  if (isset($forms[$tab])) {
    return backdrop_get_form($forms[$tab]);
  }

  return MENU_NOT_FOUND;
}

/**
 * Displays the Settings Content tab.
 */
function theme_setup_builder_settings_content_page() {
  return array(
    '#markup' => '<p>' . t('Settings Content export tools will be added here.') . '</p>',
  );
}

/**
 * Displays a Settings Content secondary tab.
 */
function theme_setup_builder_settings_tab_page($tab) {
  switch ($tab) {
    case 'modules':
      return backdrop_get_form('theme_setup_builder_modules_form');

    case 'fields':
      return backdrop_get_form('theme_setup_builder_fields_form');

    case 'taxonomy-terms':
      return backdrop_get_form('theme_setup_builder_taxonomy_terms_form');

    case 'menu-links':
      return backdrop_get_form('theme_setup_builder_menu_links_form');

    case 'sample-content':
      return backdrop_get_form('theme_setup_builder_content_form');
  }

  return MENU_NOT_FOUND;
}

/**
 * Builds the Image styles export form.
 */
function theme_setup_builder_image_styles_form($form, &$form_state) {
  $options = array();

  foreach (image_styles() as $style_name => $style) {
    $options[$style_name] = $style['label'];
  }

  return theme_setup_builder_export_form(
    $form,
    'image-styles',
    t('Image styles'),
    $options
  );
}

/**
 * Builds the Text formats export form.
 */
function theme_setup_builder_text_formats_form($form, &$form_state) {
  $options = array();

  foreach (filter_formats() as $format_name => $format) {
    $options[$format_name] = $format->name;
  }

  return theme_setup_builder_export_form(
    $form,
    'text-formats',
    t('Text formats and editors'),
    $options
  );
}

/**
 * Builds the Custom blocks export form.
 */
function theme_setup_builder_custom_blocks_form($form, &$form_state) {
  $options = array();

  $configs = config_get_names_with_prefix('block.custom.');

  foreach ($configs as $config_name) {
    $name = substr($config_name, strlen('block.custom.'));
    $data = config($config_name)->getOriginal();

    $options[$name] = !empty($data['info']) ? $data['info'] : $name;
  }

  return theme_setup_builder_export_form(
    $form,
    'custom-blocks',
    t('Custom blocks'),
    $options
  );
}

/**
 * Builds the Views export form.
 */
function theme_setup_builder_views_form($form, &$form_state) {
  $options = array();

  $configs = config_get_names_with_prefix('views.view.');

  foreach ($configs as $config_name) {
    $name = substr($config_name, strlen('views.view.'));
    $data = config($config_name)->getOriginal();

    $options[$name] = !empty($data['name']) ? $data['name'] : $name;
  }

  return theme_setup_builder_export_form(
    $form,
    'views',
    t('Views'),
    $options
  );
}

/**
 * Builds the Layouts export form.
 */
function theme_setup_builder_layouts_form($form, &$form_state) {
  $options = array();

  $configs = config_get_names_with_prefix('layout.layout.');

  foreach ($configs as $config_name) {
    $name = substr($config_name, strlen('layout.layout.'));
    $data = config($config_name)->getOriginal();

    $options[$name] = !empty($data['title']) ? $data['title'] : $name;
  }

  return theme_setup_builder_export_form(
    $form,
    'layouts',
    t('Layouts'),
    $options
  );
}

/**
 * Builds the Fields overview form.
 */
function theme_setup_builder_fields_form($form, &$form_state) {
  $instances = field_info_instances();
  $bundles = field_info_bundles();

  $form['description'] = array(
    '#markup' => '<p>' . t('Select fields to export for your theme.') . '</p>',
  );

 $form['#tree'] = TRUE;

  foreach ($instances as $entity_type => $entity_bundles) {
    // Remove bundles that have no field instances.
    $entity_bundles = array_filter($entity_bundles);

    // Do not display an entity type if it has no fields.
    if (empty($entity_bundles)) {
      continue;
    }

    $form[$entity_type] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($entity_type),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    foreach ($entity_bundles as $bundle_name => $bundle_instances) {
      if (empty($bundle_instances)) {
        continue;
      }

      $bundle_label = $bundle_name;

      if (!empty($bundles[$entity_type][$bundle_name]['label'])) {
        $bundle_label = $bundles[$entity_type][$bundle_name]['label'];
      }

      $options = array();

        foreach ($bundle_instances as $field_name => $instance) {
         $label = !empty($instance['label'])
            ? $instance['label']
            : $field_name;

         $field = field_info_field($field_name);

         $type = !empty($field['type'])
            ? $field['type']
            : 'unknown';

         $module = !empty($field['module'])
            ? $field['module']
            : 'unknown';

         $options[$field_name] = $label
            . ' (' . $field_name . ')'
            . ' — ' . $type
            . ' / ' . $module;
        }

      if (empty($options)) {
        continue;
      }

      $form[$entity_type][$bundle_name] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($bundle_label),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $form[$entity_type][$bundle_name]['fields'] = array(
        '#type' => 'checkboxes',
        '#options' => $options,
      );
    }
  }

    $form['#export_section'] = 'fields';

    $form['actions'] = array(
    '#type' => 'actions',
    );

    $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export selected'),
    '#submit' => array('theme_setup_builder_config_submit'),
    );

  return $form;
}

/**
 * Exports selected field definitions and field instances.
 */
function theme_setup_builder_export_fields(&$form_state) {
  $directory = 'public://theme_setup_builder/settings-content/fields';

  if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
    backdrop_set_message(t('Unable to prepare the fields export directory.'), 'error');
    return;
  }

  $instances = field_info_instances();
  $exported_fields = array();

  $field_count = 0;
  $instance_count = 0;

  foreach ($instances as $entity_type => $entity_bundles) {
    if (empty($form_state['values'][$entity_type])) {
      continue;
    }

    foreach ($entity_bundles as $bundle_name => $bundle_instances) {
      if (empty($form_state['values'][$entity_type][$bundle_name]['fields'])) {
        continue;
      }

      $selected = array_filter(
        $form_state['values'][$entity_type][$bundle_name]['fields']
      );

      foreach ($selected as $field_name) {
        // Make sure the selected field belongs to this bundle.
        if (!isset($bundle_instances[$field_name])) {
          continue;
        }

        // Export the field definition only once.
        if (!isset($exported_fields[$field_name])) {
          $config_name = 'field.field.' . $field_name;

          if (theme_setup_builder_export_config($config_name, $directory)) {
            $exported_fields[$field_name] = TRUE;
            $field_count++;
          }
        }

        // Export the instance for this entity type and bundle.
        $config_name = 'field.instance.'
          . $entity_type . '.'
          . $bundle_name . '.'
          . $field_name;

        if (theme_setup_builder_export_config($config_name, $directory)) {
          $instance_count++;
        }
      }
    }
  }

  if (!$field_count && !$instance_count) {
    backdrop_set_message(t('Please select at least one field.'), 'warning');
    return;
  }

  backdrop_set_message(t('Exported @fields field definitions and @instances field instances.', array(
    '@fields' => $field_count,
    '@instances' => $instance_count,
  )));
}

/**
 * Builds the modules export form.
 */
function theme_setup_builder_modules_form($form, &$form_state) {
  $form['#export_section'] = 'modules';

  $form['description'] = array(
    '#markup' => '<p><strong>' . t('Select only modules that are required by your theme or its sample content. Enabling unnecessary modules may increase resource usage and add unwanted dependencies.') . '</strong></p>',
  );

  $modules = module_list(TRUE, FALSE, TRUE);
  $options = array();

  foreach ($modules as $name => $module) {
    $label = !empty($module->info['name'])
      ? $module->info['name']
      : $name;

    $options[$name] = $label . ' (' . $name . ')';
  }

  natcasesort($options);

  $form['modules'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Required modules'),
    '#options' => $options,
  );

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export selected'),
    '#submit' => array('theme_setup_builder_config_submit'),
  );

  return $form;
}

/**
 * Exports selected required modules to modules.json.
 */
function theme_setup_builder_export_modules(&$form_state) {
  $selected = !empty($form_state['values']['modules'])
    ? array_filter($form_state['values']['modules'])
    : array();

  if (empty($selected)) {
    backdrop_set_message(t('Please select at least one module.'), 'warning');
    return;
  }

  $directory = 'public://theme_setup_builder/settings-content/modules';

  $data = array(
    'modules' => array_values($selected),
  );

  if (!theme_setup_builder_write_json($data, $directory, 'modules.json', array(
    'directory' => t('Unable to prepare the modules export directory.'),
    'encode' => t('Unable to encode the selected modules.'),
    'save' => t('Unable to export modules.json.'),
  ))) {
    return;
  }

  backdrop_set_message(t('Exported @count required modules to modules.json.', array(
    '@count' => count($selected),
  )));
}

/**
 * Builds the menu links export form.
 */
function theme_setup_builder_menu_links_form($form, &$form_state) {
  $form['#export_section'] = 'menu-links';
  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#markup' => '<p>' . t('Select the menu links to include with your theme setup.') . '</p>',
  );

  $menus = menu_get_menus();

  foreach ($menus as $menu_name => $menu_title) {
    $tree = menu_tree_all_data($menu_name);

    if (empty($tree)) {
      continue;
    }

    $options = array();
    theme_setup_builder_collect_menu_links($tree, $options);

    if (empty($options)) {
      continue;
    }

    $form['menus'][$menu_name] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($menu_title),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['menus'][$menu_name]['links'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export selected'),
    '#submit' => array('theme_setup_builder_config_submit'),
  );

  return $form;
}

/**
 * Collects menu links recursively for the export form.
 */
function theme_setup_builder_collect_menu_links(array $tree, array &$options, $depth = 0) {
  foreach ($tree as $item) {
    if (empty($item['link']['mlid'])) {
      continue;
    }

    $link = $item['link'];
    $prefix = str_repeat('— ', $depth);

    $options[$link['mlid']] = $prefix . $link['link_title'];

    if (!empty($item['below'])) {
      theme_setup_builder_collect_menu_links(
        $item['below'],
        $options,
        $depth + 1
      );
    }
  }
}

/**
 * Exports selected menu links to links.json.
 */
function theme_setup_builder_export_menu_links(&$form_state) {
  if (empty($form_state['values']['menus'])) {
    backdrop_set_message(t('Please select at least one menu link.'), 'warning');
    return;
  }

  $links = array();

  foreach ($form_state['values']['menus'] as $menu_name => $menu_values) {
    if (empty($menu_values['links'])) {
      continue;
    }

    $selected = array_filter($menu_values['links']);

    foreach ($selected as $mlid) {
      $link = menu_link_load($mlid);

      if (empty($link)) {
        continue;
      }

      $item = array(
        'menu' => $menu_name,
        'title' => $link['link_title'],
        'path' => $link['link_path'],
        'weight' => (int) $link['weight'],
        'expanded' => !empty($link['expanded']),
        'hidden' => !empty($link['hidden']),
      );

        // Add the parent title for child links.
        if (!empty($link['plid'])) {
        $parent = menu_link_load($link['plid']);

        if (!empty($parent['link_title'])) {
            $item['parent'] = $parent['link_title'];
        }
        }

      $links[] = $item;
    }
  }

  if (empty($links)) {
    backdrop_set_message(t('Please select at least one menu link.'), 'warning');
    return;
  }

  $directory = 'public://theme_setup_builder/settings-content/menu';

  $data = array(
    'links' => $links,
  );

  if (!theme_setup_builder_write_json($data, $directory, 'links.json', array(
    'directory' => t('Unable to prepare the menu export directory.'),
    'encode' => t('Unable to encode the selected menu links.'),
    'save' => t('Unable to export links.json.'),
  ))) {
    return;
  }

  backdrop_set_message(t('Exported @count menu links to links.json.', array(
    '@count' => count($links),
  )));
}

/**
 * Builds the taxonomy terms export form.
 */
function theme_setup_builder_taxonomy_terms_form($form, &$form_state) {
  $form['#export_section'] = 'taxonomy-terms';
  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#markup' => '<p>' . t('Select the taxonomy terms to include with your theme setup.') . '</p>',
  );

  $vocabularies = taxonomy_get_vocabularies();

  foreach ($vocabularies as $vocabulary) {
    $terms = taxonomy_get_tree($vocabulary->machine_name);

    if (empty($terms)) {
      continue;
    }

    $options = array();

    foreach ($terms as $term) {
      $prefix = str_repeat('— ', $term->depth);
      $options[$term->tid] = $prefix . $term->name;
    }

    $form['vocabularies'][$vocabulary->machine_name] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($vocabulary->name),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['vocabularies'][$vocabulary->machine_name]['terms'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export selected'),
    '#submit' => array('theme_setup_builder_config_submit'),
  );

  return $form;
}

/**
 * Exports selected taxonomy terms to terms.json.
 */
function theme_setup_builder_export_taxonomy_terms(&$form_state) {
  if (empty($form_state['values']['vocabularies'])) {
    backdrop_set_message(t('Please select at least one taxonomy term.'), 'warning');
    return;
  }

  $terms = array();

  foreach ($form_state['values']['vocabularies'] as $vocabulary_name => $values) {
    if (empty($values['terms'])) {
      continue;
    }

    $selected = array_filter($values['terms']);

    foreach ($selected as $tid) {
      $term = taxonomy_term_load($tid);

      if (empty($term)) {
        continue;
      }

      $item = array(
        'vocabulary' => $vocabulary_name,
        'name' => $term->name,
        'description' => !empty($term->description) ? $term->description : '',
        'weight' => isset($term->weight) ? (int) $term->weight : 0,
        'path' => '',
      );

      // Export the term URL alias when available.
      $alias = backdrop_get_path_alias('taxonomy/term/' . $term->tid);

      if ($alias !== 'taxonomy/term/' . $term->tid) {
        $item['path'] = $alias;
      }

        // Add the parent term name for child terms.
        $parents = taxonomy_term_load_parents($term->tid);

        if (!empty($parents)) {
        $parent = reset($parents);

        if (!empty($parent->name)) {
            $item['parent'] = $parent->name;
        }
        }
      $terms[] = $item;
    }
  }

  if (empty($terms)) {
    backdrop_set_message(t('Please select at least one taxonomy term.'), 'warning');
    return;
  }

  $directory = 'public://theme_setup_builder/settings-content/taxonomy';

  $data = array(
    'terms' => $terms,
  );

  if (!theme_setup_builder_write_json($data, $directory, 'terms.json', array(
    'directory' => t('Unable to prepare the taxonomy export directory.'),
    'encode' => t('Unable to encode the selected taxonomy terms.'),
    'save' => t('Unable to export terms.json.'),
  ))) {
    return;
  }

  backdrop_set_message(t('Exported @count taxonomy terms to terms.json.', array(
    '@count' => count($terms),
  )));
}

/**
 * Builds the sample content export form.
 */
function theme_setup_builder_content_form($form, &$form_state) {
  $form['#export_section'] = 'content';
  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#markup' => '<p>' . t('Select only the content that should be included as sample content with your theme.') . '</p>',
  );

  $types = node_type_get_types();

  foreach ($types as $type_name => $type) {
    $query = new EntityFieldQuery();

    $result = $query
      ->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', $type_name)
      ->execute();

    if (empty($result['node'])) {
      continue;
    }

    $options = array();

    $nodes = node_load_multiple(array_keys($result['node']));

    foreach ($nodes as $node) {

    if (!$node) {
        continue;
    }

    $options[$node->nid] = $node->title;
    }

    if (empty($options)) {
      continue;
    }

    asort($options);

    $form['types'][$type_name] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($type->name),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['types'][$type_name]['nodes'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
    );
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export selected'),
    '#submit' => array('theme_setup_builder_config_submit'),
  );

  return $form;
}

/**
 * Prepares a field value for Theme Setup content export.
 */
function theme_setup_builder_prepare_field($field_name, $items) {
  $field = field_info_field($field_name);

  if (!$field || empty($items)) {
    return NULL;
  }

  $type = $field['type'];
  $values = array();

  foreach ($items as $item) {
    switch ($type) {
    case 'text':
    case 'text_long':
    case 'text_with_summary':
        $values[] = isset($item['value']) ? $item['value'] : '';
        break;

    case 'number_integer':
        $values[] = isset($item['value']) ? (int) $item['value'] : 0;
        break;

    case 'number_decimal':
    case 'number_float':
        $values[] = isset($item['value']) ? (float) $item['value'] : 0;
        break;

    case 'list_boolean':
        $values[] = !empty($item['value']) ? 1 : 0;
        break;

    case 'list_integer':
        $values[] = isset($item['value']) ? (int) $item['value'] : 0;
        break;

    case 'list_float':
        $values[] = isset($item['value']) ? (float) $item['value'] : 0;
        break;

    case 'list_text':
        $values[] = isset($item['value']) ? $item['value'] : '';
        break;

      case 'taxonomy_term_reference':
        if (empty($item['tid'])) {
          continue 2;
        }

        $term = taxonomy_term_load($item['tid']);

        if (!$term) {
          continue 2;
        }

        $values[] = array(
          'vocabulary' => $term->vocabulary,
          'name' => $term->name,
        );
        break;

      case 'entityreference':
        if (empty($item['target_id'])) {
          continue 2;
        }

        $referenced = node_load($item['target_id']);

        if (!$referenced) {
          continue 2;
        }

        $values[] = array(
          'type' => $referenced->type,
          'title' => $referenced->title,
        );
        break;

      case 'image':
      case 'file':
        if (empty($item['fid'])) {
          continue 2;
        }

        $file = file_load($item['fid']);

        if (!$file) {
          continue 2;
        }

        if (!theme_setup_builder_export_file($file)) {
          continue 2;
       }

        $values[] = backdrop_basename($file->uri);
        break;

      default:
        $values[] = $item;
        break;
    }
  }

  if (empty($values)) {
    return NULL;
  }

  return count($values) === 1 ? reset($values) : $values;
}

/**
 * Exports selected sample content.
 */
function theme_setup_builder_export_content(&$form_state) {
  if (empty($form_state['values']['types'])) {
    backdrop_set_message(t('Please select at least one content item.'), 'warning');
    return;
  }

  $content = array();

  foreach ($form_state['values']['types'] as $type_name => $values) {
    if (empty($values['nodes'])) {
      continue;
    }

    $selected = array_filter($values['nodes']);

    foreach ($selected as $nid) {
      $node = node_load($nid);

      if (!$node) {
        continue;
      }

      $item = array(
        'type' => $node->type,
        'title' => $node->title,
        'path' => '',
        'status' => (int) $node->status,
        'promote' => (int) $node->promote,
      );

      // Export the URL alias when available.
      $alias = backdrop_get_path_alias('node/' . $node->nid);

      if ($alias !== 'node/' . $node->nid) {
        $item['path'] = $alias;
      }

      // Export body.
      $body_items = field_get_items('node', $node, 'body', LANGUAGE_NONE);

      if (!empty($body_items[0])) {
        $body = $body_items[0];

        $item['body'] = array(
          'value' => isset($body['value']) ? $body['value'] : '',
          'format' => isset($body['format']) ? $body['format'] : NULL,
        );
      }

      // Export fields.
      $fields = field_info_instances('node', $node->type);

      foreach ($fields as $field_name => $instance) {
        if ($field_name === 'body') {
          continue;
        }

        $field_items = field_get_items(
          'node',
          $node,
          $field_name,
          LANGUAGE_NONE
        );

        if (empty($field_items)) {
          continue;
        }

        $value = theme_setup_builder_prepare_field(
          $field_name,
          $field_items
        );

        if ($value !== NULL) {
          $item['fields'][$field_name] = $value;
        }
      }

      $content[] = $item;
    }
  }

  if (empty($content)) {
    backdrop_set_message(t('Please select at least one content item.'), 'warning');
    return;
  }

  $directory = 'public://theme_setup_builder/settings-content/content';

  $data = array(
    'content' => $content,
  );

  if (!theme_setup_builder_write_json($data, $directory, 'content.json', array(
    'directory' => t('Unable to prepare the content export directory.'),
    'encode' => t('Unable to encode the selected content.'),
    'save' => t('Unable to export content.json.'),
  ))) {
    return;
  }

  backdrop_set_message(t('Exported @count content items to content.json.', array(
    '@count' => count($content),
  )));
}

function theme_setup_builder_export_file($file) {
  static $exported_files = array();
  static $directory_ready = FALSE;

  if (!empty($file->fid) && isset($exported_files[$file->fid])) {
    return $exported_files[$file->fid];
  }

  if (empty($file->uri) || !file_exists($file->uri)) {
    return FALSE;
  }

  $directory = 'public://theme_setup_builder/settings-content/files';

  if (!$directory_ready) {
    if (!file_prepare_directory($directory, FILE_CREATE_DIRECTORY)) {
      return FALSE;
    }
    $directory_ready = TRUE;
  }

  $destination = $directory . '/' . backdrop_basename($file->uri);

  $success = file_unmanaged_copy(
    $file->uri,
    $destination,
    FILE_EXISTS_REPLACE
  ) !== FALSE;

  if ($success && !empty($file->fid)) {
    $exported_files[$file->fid] = TRUE;
  }

  return $success;
}

