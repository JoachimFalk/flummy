// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.helper;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.lang.reflect.InvocationTargetException;
import java.util.Map.Entry;
import java.util.Vector;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import org.opt4j.core.config.Property;

@SuppressWarnings("serial")
public class ObjectivesPanel
    extends
        JScrollPane
    implements
        ActionListener
      , TableModelListener
{

    public ObjectivesPanel(Property property) {
        this.property = property;
        this.environment = (Objectives) property.getValue();

        tableModel = new DefaultTableModel(
                new Object[]{"Objectives variable", "value"}, 0);
        for (Entry<String, String> e : environment.entrySet()) {
            tableModel.addRow(new Object[]{e.getKey(), e.getValue()});
        }
//      tableModel.addRow(new Object[]{"foo", "bar"});
        tableModel.addTableModelListener(this);
        table = new JTable(tableModel);
        this.setViewportView(table);

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd1 = new JMenuItem("New variable");
            menuItemAdd1.addActionListener(this);
            popupMenu.add(menuItemAdd1);
            menuItemRemove1 = new JMenuItem("Remove selected variables");
            menuItemRemove1.addActionListener(this);
            popupMenu.add(menuItemRemove1);
            // Set the popup menu for the table
            table.setComponentPopupMenu(popupMenu);
        }

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd2 = new JMenuItem("New variable");
            menuItemAdd2.addActionListener(this);
            popupMenu.add(menuItemAdd2);
            menuItemRemove2 = new JMenuItem("Remove selected variables");
            menuItemRemove2.addActionListener(this);
            popupMenu.add(menuItemRemove2);
            // Set the popup menu for the table
            this.setComponentPopupMenu(popupMenu);
        }

//      System.err.println(table.getMinimumSize());
//      System.err.println(table.getPreferredSize());
//      System.err.println(scrollPane.getMinimumSize());
//      System.err.println(scrollPane.getPreferredSize());
        this.setMinimumSize(new Dimension(-1, 22*4));
        this.setPreferredSize(this.getMinimumSize());

    }

    private final Property property;
    private final Objectives environment;
    private final DefaultTableModel tableModel;
    private final JTable table;
    private final JMenuItem menuItemAdd1, menuItemAdd2;
    private final JMenuItem menuItemRemove1, menuItemRemove2;

    @Override
    public void actionPerformed(ActionEvent event) {
        JMenuItem menu = (JMenuItem) event.getSource();
        if (menu == menuItemAdd1 || menu == menuItemAdd2) {
            addNewRow();
        } else if (menu == menuItemRemove1 || menu == menuItemRemove2) {
            removeSelectedRows();
        }
    }

    private void addNewRow() {
        tableModel.addRow(new String[0]);
    }

    private void removeSelectedRows() {
        int[] selectedRows = table.getSelectedRows();

        for (int i = selectedRows.length-1; i >= 0; --i) {
            tableModel.removeRow(selectedRows[i]);
        }
    }

    @Override
    public void tableChanged(TableModelEvent e) {
//      System.err.println(e);
        environment.clear();
        @SuppressWarnings("unchecked")
        Vector<Vector<String>> rows = tableModel.getDataVector();
//      System.err.println(rows);
        for (Vector<String> row : rows) {
            final String var   = row.get(0);
            final String value = row.get(1);
            if (var != null) {
                environment.put(
                    var
                  , value != null ? value : "");
            }
        }
        try {
            property.setValue(environment);
        } catch (InvocationTargetException e1) {
            e1.printStackTrace();
        }
    }

}
